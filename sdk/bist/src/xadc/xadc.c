/******************************************************************************
 * @file xadc.c
 *
 * @authors Elod Gyorgy
 *
 * @date 2014-Dec-29
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
 * 1.00  Elod Gyorgy  2014-Dec-29 First release
 *
 * </pre>
 *
 *****************************************************************************/

#include "xadc.h"
#include "xparameters.h"

#define XADC_DEVICE_ID	XPAR_SYSMON_0_DEVICE_ID

/*
 *  Function: fnXADCInit
 *      Initialize the XADC core.
 *
 *	Parameters:
 *      XSysMon *psXADC	- pointer to driver instance allocated by caller
 *
 *  Returns:
 *      Zero on sucess, non-zero on failure
 *
*/
XStatus fnXADCInit(XSysMon *psXADC)
{
	XSysMon_Config *ConfigPtr;
	XStatus Status;

	ConfigPtr = XSysMon_LookupConfig(XADC_DEVICE_ID);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XSysMon_CfgInitialize(psXADC, ConfigPtr, ConfigPtr->BaseAddress);

	/*
	 * Self Test the System Monitor/ADC device
	 */
	RETURN_ON_FAILURE(XSysMon_SelfTest(psXADC));

	/*
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	XSysMon_SetSequencerMode(psXADC, XSM_SEQ_MODE_SAFE);

	/*
	 * Enable the averaging on the following channels in the Sequencer
	 * registers:
	 * 	- On-chip Temperature
	 */
	RETURN_ON_FAILURE(XSysMon_SetSeqAvgEnables(psXADC, XSM_SEQ_CH_TEMP));

	/*
	 * Enable the following channels in the Sequencer registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCINT supply sensor
	 * 	- On-chip VCCAUX supply sensor
	 */
	RETURN_ON_FAILURE(XSysMon_SetSeqChEnables(psXADC, XSM_SEQ_CH_TEMP |
			XSM_SEQ_CH_VCCINT | XSM_SEQ_CH_VCCAUX));

	/*
	 * Set the ADCCLK frequency equal to 100/4=25MHz.
	 */
	XSysMon_SetAdcClkDivisor(psXADC, 4);

	/*
	 * Disable all the alarms in the Configuration Register 1
	 */
	XSysMon_SetAlarmEnables(psXADC, 0x0);

	/*
	 * Set up Alarm threshold registers for
	 * On-chip Temperature High limit
	 * On-chip Temperature Low limit
	 */
	XSysMon_SetAlarmThreshold(psXADC, XSM_ATR_TEMP_UPPER, XSysMon_TemperatureToRaw(90));
	XSysMon_SetAlarmThreshold(psXADC, XSM_ATR_TEMP_LOWER, XSysMon_TemperatureToRaw(0));

	/*
	 * Enable Alarm 0 for on-chip temperature and Alarm 2 for on-chip
	 * VCCAUX in the Configuration Register 1.
	 */
	XSysMon_SetAlarmEnables(psXADC, XSM_CFR1_ALM_TEMP_MASK);

	// We have MIG in this design, so make sure temperature readings are sent to it
	XSysMon_EnableTempUpdate(psXADC);

	/*
	 * Enable the Channel Sequencer in continuous sequencer cycling mode.
	 */
	XSysMon_SetSequencerMode(psXADC, XSM_SEQ_MODE_CONTINPASS);

	return XST_SUCCESS;
}


