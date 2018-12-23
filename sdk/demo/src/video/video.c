/******************************************************************************
 * @file video.c
 *
 * @authors Elod Gyorgy
 *
 * @date 2015-Jan-15
 *
 * @copyright
 * (c) 2015 Copyright Digilent Incorporated
 * All Rights Reserved
 *
 * This program is free software; distributed under the terms of BSD 3-clause
 * license ("Revised BSD License", "New BSD License", or "Modified BSD License")
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name(s) of the above-listed copyright holder(s) nor the names
 *    of its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 * @desciption
 * Contains init functions for video path multiplexers, mouse overlay, test
 * pattern generator and video timing controller
 * @note
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who          Date        Changes
 * ----- ------------ ----------- --------------------------------------------
 * 1.00  Elod Gyorgy  2015-Jan-15 First release
 *
 * </pre>
 *
 *****************************************************************************/

#include "video.h"
#include "xparameters.h"
#include "../demo.h"


#define TPG_DEVICE_ID	XPAR_VIDEO_V_TPG_0_DEVICE_ID
#define PS2_MOUSE_BASEADDR	XPAR_VIDEO_PS2_MOUSE_OVERLAY_0_BASEADDR

extern sDemo_t Demo;

void fnSwitchVideoPathIfNoClock(XGpio *psGpio);

/*
 *  Function: fnVideoPathCtrlHandler
 *      Video Path Interrupt Service Routine
 *
 *	Parameters:
 *      void *CallbackRef - XGpio* type reference to the GPIO driver instance
 *      controlling the video path.
 *
*/
void fnVideoPathCtrlIsr(void *CallbackRef)
{
	XGpio *psGpio = (XGpio *)CallbackRef;

	// Disable the interrupt
	XGpio_InterruptGlobalDisable(psGpio);

	// Make sure the interrupt is on the input channel
	if ((XGpio_InterruptGetStatus(psGpio) & VIDEO_PATH_CTRL_INTERRUPT) != VIDEO_PATH_CTRL_INTERRUPT) {
		return;
	}

	fnSwitchVideoPathIfNoClock(psGpio);

	Demo.fVideoEvent = 1;

	// Clear the Interrupt
	XGpio_InterruptClear(psGpio, VIDEO_PATH_CTRL_INTERRUPT);

	// Enable the interrupt
	XGpio_InterruptGlobalEnable(psGpio);
}

/*
 *  Function: fnPS2MouseShow
 *      Enables/disables mouse overlay on video output.
 *
 *	Parameters:
 *      int fVisible = 0 to disable the mouse pointer, any other value to enable
 *
 *
*/
void fnPS2MouseShow(int fVisible)
{
	Xil_Out32(PS2_MOUSE_BASEADDR + 0x0, fVisible ? 1 : 0);
}

/*
 *  Function: fnTpgInit
 *      Initialize the Test Pattern Generator core and set it to a default pattern.
 *
 *	Parameters:
 *      XTpg *psTpg	- pointer to driver instance allocated by caller
 *
 *  Returns:
 *      Zero on sucess, non-zero on failure
 *
*/
XStatus fnTpgInit(XTpg *psTpg)
{
	XTpg_Config *psTpgConfig;
	XStatus Status;

	psTpgConfig = XTpg_LookupConfig(TPG_DEVICE_ID);
	if (NULL == psTpgConfig) {
		return XST_FAILURE;
	}

	Status = XTpg_CfgInitialize(psTpg, psTpgConfig, psTpgConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Reset Test Pattern Generator
	XTpg_Reset(psTpg);

	// Set pattern characteristics
	XTpg_SetActiveSize(psTpg, H_ACTIVE_SIZE, V_ACTIVE_SIZE);
	XTpg_SetPattern(psTpg, INIT_PATTERN);
	XTpg_EnableMotion(psTpg);
	XTpg_SetMotionSpeed(psTpg, 1);
	XTpg_RegUpdateEnable(psTpg);

	// Go
	XTpg_Enable(psTpg);

	return XST_SUCCESS;
}

/*
 *  Function: fnVtdInit
 *      Initialize the Video Timing Controller used as a detector on the DVI Input.
 *
 *	Parameters:
 *      XTpg *psTpg	- pointer to driver instance allocated by caller
 *
 *  Returns:
 *      Zero on sucess, non-zero on failure
 *
*/
XStatus fnVtdInit(XVtc *psVtcInst) {

	XStatus Status;
	XVtc_Config *psVtcConfig;

	// Initializing VTC
	psVtcConfig = XVtc_LookupConfig(XPAR_VTC_0_DEVICE_ID);
	if(NULL == psVtcConfig) {
		return XST_FAILURE;
	}

	Status = XVtc_CfgInitialize(psVtcInst, psVtcConfig, psVtcConfig->BaseAddress);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Enabling the VTC Detector
	XVtc_EnableDetector(psVtcInst);

	return XST_SUCCESS;
}

/*
 *  Function: fnInitVidMux
 *      Initialize the video path control unit. This will request interrupt on DVI input
 *      clock lock. We can also control the two video selection muxes for DVI and DP output
 *
 *	Parameters:
 *      XGpio *psGpio - pointer to driver instance allocated by caller
 *
 *  Returns:
 *      Zero on sucess, non-zero on failure
 *
*/
XStatus fnInitVidMux(XGpio *psGpio)
{
	XGpio_Config *psGpioConfig;
	XStatus Status;

	psGpioConfig = XGpio_LookupConfig(VIDEO_PATH_CTRL_ID);
	if (NULL == psGpioConfig) {
		return XST_FAILURE;
	}

	/*
	 * Initialize the GPIO driver
	 */
	Status = XGpio_CfgInitialize(psGpio, psGpioConfig, psGpioConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Run self-test
	Status = XGpio_SelfTest(psGpio);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Enable interrupt on the input channel
	XGpio_InterruptEnable(psGpio, VIDEO_PATH_CTRL_INTERRUPT);
	XGpio_InterruptGlobalEnable(psGpio);

	return XST_SUCCESS;
}

/*
 *  Function: fnSetMux
 *      Initialize the Test Pattern Generator core and set it to a default pattern.
 *
 *	Parameters:
 *      XGpio *psGpio - pointer to driver instance allocated by caller
 *      int dMuxId - id of the mux
 *
 *
*/
void fnSetMux(XGpio *psGpio, int dMuxId, int dInput)
{
	// We only have two muxes, on bit positions 0 and 1
	Xil_AssertVoid((1 << dMuxId) & 0x3);

	if (dInput) {
		XGpio_DiscreteSet(psGpio, VIDEO_PATH_CTRL_OUTPUT_CHANNEL, (1 << dMuxId) & 0x3);
	}
	else {
		XGpio_DiscreteClear(psGpio, VIDEO_PATH_CTRL_OUTPUT_CHANNEL, (1 << dMuxId) & 0x3);
	}
}

/*
 *  Function: fnSwitchVideoPathIfNoClock
 *      Switches both video outputs to pattern, if no DVI Input is detected
 *
 *	Parameters:
 *      XGpio *psGpio - pointer to driver instance allocated by caller
 *
 *
*/
void fnSwitchVideoPathIfNoClock(XGpio *psGpio)
{

	// Checking the status of the Lock bit of DVI2RGB core
	if((XGpio_DiscreteRead(psGpio, VIDEO_PATH_CTRL_INPUT_CHANNEL) &
			VIDEO_PATH_CTRL_DVICLOCK_LOCKED_BIT_MASK)) {
		//DVI Input, switch to it
		fnSwitchDVItoDVI(psGpio);
		fnSwitchDPtoDVI(psGpio);
		Demo.fDVIClockLock = 1;
	}
	else {
		//No DVI Input, switch to pattern
		fnSwitchDVItoPattern(psGpio);
		fnSwitchDPtoPattern(psGpio);
		Demo.fDVIClockLock = 0;
	}
}
