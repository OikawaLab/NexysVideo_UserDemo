/******************************************************************************
 * @file userio.c
 *
 * @authors Elod Gyorgy
 *
 * @date 2015-Jan-03
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
 *
 * @note
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who          Date        Changes
 * ----- ------------ ----------- --------------------------------------------
 * 1.00  Elod Gyorgy  2015-Jan-03 First release
 *
 * </pre>
 *
 *****************************************************************************/

#include <stdio.h>
#include "xparameters.h"
#include "userio.h"

#define USERIO_DEVICE_ID 	XPAR_USERIO_DEVICE_ID

extern unsigned char u8UserIO;

void fnUpdateLedsFromSwitches(XGpio *psGpio);

XStatus fnInitUserIO(XGpio *psGpio)
{
	/* Initialize the GPIO driver. If an error occurs then exit */
	RETURN_ON_FAILURE(XGpio_Initialize(psGpio, USERIO_DEVICE_ID));

	/*
	 * Perform a self-test on the GPIO.  This is a minimal test and only
	 * verifies that there is not any bus error when reading the data
	 * register
	 */
	RETURN_ON_FAILURE(XGpio_SelfTest(psGpio));

	/*
	 * Setup direction register so the switches and buttons are inputs and the LED is
	 * an output of the GPIO
	 */
	XGpio_SetDataDirection(psGpio, BTN_SW_CHANNEL, BTNS_SWS_MASK);
	XGpio_SetDataDirection(psGpio, LED_CHANNEL, ~LEDS_MASK);

	fnUpdateLedsFromSwitches(psGpio);

	/*
	 * Enable the GPIO channel interrupts so that push button can be
	 * detected and enable interrupts for the GPIO device
	 */
	XGpio_InterruptEnable(psGpio, BTN_SW_INTERRUPT);
	XGpio_InterruptGlobalEnable(psGpio);

	return XST_SUCCESS;
}

void fnUpdateLedsFromSwitches(XGpio *psGpio)
{
	static u32 dwPrevSwitches = 0;
	static u32 dwPrevButtons = 0;
	u32 dwSw, dwBtn;
	u32 dwBtnSw;

	dwBtnSw = XGpio_DiscreteRead(psGpio, BTN_SW_CHANNEL);
	dwSw = dwBtnSw & SWS_MASK;
	dwBtn = dwBtnSw & (BTNU_MASK|BTNR_MASK|BTND_MASK|BTNL_MASK|BTNC_MASK);

	// Has anything changed?
	if (dwSw ^ dwPrevSwitches || dwBtn ^ dwPrevButtons)
	{
		// Light up LEDs
		XGpio_DiscreteWrite(psGpio, LED_CHANNEL, dwBtn ? ~dwSw : dwSw);

		// See if we need to send status on UART
		if (u8UserIO)
		{
			int i;
			u32 dwChanges = dwSw ^ dwPrevSwitches;
			for (i = 0; i < 8; i++)
			{
				if (dwChanges & 0x1) {
					xil_printf("\r\nSW%d",i);
				}
				dwChanges >>= 1;
			}
			dwChanges = dwBtn ^ dwPrevButtons;
			if (dwChanges & BTNU_MASK) print("\r\nBTNU");
			if (dwChanges & BTNR_MASK) print("\r\nBTNR");
			if (dwChanges & BTND_MASK) print("\r\nBTND");
			if (dwChanges & BTNL_MASK) print("\r\nBTNL");
			if (dwChanges & BTNC_MASK) print("\r\nBTNC");
		}

		// Keep values in mind
		dwPrevSwitches = dwSw;
		dwPrevButtons = dwBtn;
	}
}

/*
 * Default interrupt service routine
 * Lights up LEDs above active switches. Pressing any of the buttons inverts LEDs.
 */
void fnUserIOIsr(void *pvInst)
{
	XGpio *psGpio = (XGpio*)pvInst;

	/*
	 * Disable the interrupt
	 */
	XGpio_InterruptGlobalDisable(psGpio);

	/*
	 * Check if the interrupt interests us
	 */
	if ((XGpio_InterruptGetStatus(psGpio) & BTN_SW_INTERRUPT) !=
			BTN_SW_INTERRUPT) {
		XGpio_InterruptGlobalEnable(psGpio);
		return;
	}

	fnUpdateLedsFromSwitches(psGpio);

	 /* Clear the interrupt such that it is no longer pending in the GPIO */
	XGpio_InterruptClear(psGpio, BTN_SW_INTERRUPT);

	/*
	* Enable the interrupt
	*/
	XGpio_InterruptGlobalEnable(psGpio);
}
