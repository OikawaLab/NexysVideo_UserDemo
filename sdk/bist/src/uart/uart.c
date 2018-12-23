/******************************************************************************
 * @file uart.c
 *
 * This file contains the functions needed to test and initialize the Uart
 * controller.
 *
 * @authors Mihaita Nagy
 *
 * @date 2014-Sept-09
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
 * 1.00  Mihaita Nagy 2014-Sep-25 First release
 * 1.01  Mihaita Nagy 2014-Dec-09 Modified it for Atlys 2
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "uart.h"

/************************** Variable Definitions *****************************/
extern unsigned char u8Verbose;

/************************** Constant Definitions *****************************/
#define UART_BAUD 115200

/************************** Function Definitions *****************************/

/******************************************************************************
 * Initializes the UART-PS controller.
 *
 * @param	DeviceId is the controller's ID (from xparameters_ps.h)
 *
 * @return	none.
 *****************************************************************************/
XStatus fnUartInit(XUartNs550 *UartInst) {

	XUartNs550_Config *Config;
	XStatus Status;

	/*
	 * Initialize the UART driver so that it's ready to use.
	 * Look up the configuration in the config. table then initialize it.
	 */
	Config = XUartNs550_LookupConfig(XPAR_AXI_UART16550_0_DEVICE_ID);
	if(NULL == Config) {
		return XST_FAILURE;
	}

	Status = XUartNs550_CfgInitialize(UartInst, Config, Config->BaseAddress);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XUartNs550_SetBaud(UartInst->BaseAddress, XPAR_XUARTNS550_CLOCK_HZ, UART_BAUD);
	XUartNs550_SetLineControlReg(UartInst->BaseAddress, XUN_LCR_8_DATA_BITS);

	Status = XUartNs550_SelfTest(UartInst);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************
 * Tests the UART controller by waiting for one character to be received and
 * then sending it back.
 *
 * @param	none.
 *
 * @return	none.
 *****************************************************************************/
XStatus fnUartTest(XUartNs550 *UartInst) {

	unsigned char iK=0;
	char chRx;

	while(!XUartNs550_IsReceiveData(UartInst->BaseAddress)) {
		iK++;
		MB_Sleep(2);
		if(iK == 10) {
			xil_printf("\r\nUart Test: FAIL: receive timeout");
			return XST_FAILURE;
		}
	}

	chRx = XUartNs550_RecvByte(UartInst->BaseAddress);
	XUartNs550_SendByte(UartInst->BaseAddress, chRx);

	return XST_SUCCESS;
}
