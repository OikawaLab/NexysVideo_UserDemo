/******************************************************************************
 * @file main.c
 * This is the BT-companion embedded software used in manufacturing test for
 * the Digilent Nexys-VIDEO.
 *
 * @authors Elod Gyorgy, Mihaita Nagy
 *
 * @date 2015-Jan-23
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
 * It is based on decoding the UART-received commands and calling the appropriate
 * function that does the requested test. The test result will be sent back on
 * UART.
 *
 * @note
 *
 * UART setup:		In order to successfully communicate with this demo you
 * 					must set your terminal to 115200 Baud, 8 data bits, 1 stop
 * 					bit, no parity.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who          Date        Changes
 * ----- ------------ ----------- --------------------------------------------
 * 1.02	 elodg		  2015-Jan23  Increased MB_Sleep delays due to cache
 * 1.01	 elodg		  2015-Jan-16 Added MAC read from EEPROM
 * 1.00  Mihaita Nagy 2015-Jan-15 First release
 *  	 Elod Gyorgy
 *
 * </pre>
 *
 *****************************************************************************/
/***************************** Include Files *********************************/
#include <stdio.h>
#include <string.h>
#include <xstatus.h>
#include "xparameters.h"
#include "xil_types.h"

#include "dp/dp.h"
#include "oled/oled.h"
#include "sd/sd.h"
#include "uart/uart.h"
#include "video/video.h"
#include "audio/audio.h"
#include "xadc/xadc.h"
#include "userio/userio.h"
#include "intc/intc.h"
#include "lwipdemo/eth.h"
#include "iic/iic.h"

#include "netif/xadapter.h"
#include "lwipdemo/platform.h"

/************************** Constant Definitions *****************************/
#define MAX_BIST_CMD_LENGTH		15

// main bist states
enum bistStatus {
	STATUS_BIST_NOT_STARTED_IDLE,
	STATUS_BIST_STARTED_IDLE,
	STATUS_COMMAND_RECEIVED,
	STATUS_UNRECOGNIZED_COMMAND
};

// bist uart commands
typedef enum {
	INVALID,
	TEST_UART,
	TEST_QSPI,
	TEST_ETH_START,
	TEST_ETH_STOP,
	TEST_DP,
	TEST_SD,
	TEST_AUDIO,
	TEST_USERIO_START,
	TEST_USERIO_STOP,
	VERBOSE_ON,
	VERBOSE_OFF
} cmds_t;

// bist command map type definition
typedef struct {
	char *pchCmd;
	cmds_t eCmd;
} cmdMap_t;

// map bist commands to the corresponding string
const cmdMap_t bistCommands[] = {
	{"uart",        	TEST_UART},
	{"qspi",        	TEST_QSPI},
	{"eth-start",   	TEST_ETH_START},
	{"eth-stop",    	TEST_ETH_STOP},
	{"dp",          	TEST_DP},
	{"sd",         		TEST_SD},
	{"audio-ll",    	TEST_AUDIO},
	{"userio-start", 	TEST_USERIO_START},
	{"userio-stop", 	TEST_USERIO_STOP},
	{"verbose-on",  	VERBOSE_ON},
	{"verbose-off", 	VERBOSE_OFF}
};

const sPixmap_t sLeftCurtain = {{
	// line 1
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 1
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 2
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 3
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 4
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 5
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 6
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 7
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 9
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 10
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 11
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 12
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 13
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 14
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 15
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 16
	// line 2
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 1
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 2
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 3
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 4
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 5
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 6
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 7
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 9
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 10
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 11
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 12
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 13
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 14
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 15
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 16
	// line 3
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 1
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 2
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 3
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 4
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 5
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 6
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 7
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 9
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 10
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 11
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 12
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 13
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 14
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 15
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 16
	// line 4
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 1
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 2
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 3
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 4
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 5
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 6
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 7
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 9
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 10
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 11
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 12
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 13
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 14
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 15
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 16
}};

const sPixmap_t sRightCurtain = {{
	// line 1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 2
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 3
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 4
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 5
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 6
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 9
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 10
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 11
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 12
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 13
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 14
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 15
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 16
	// line 2
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 2
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 3
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 4
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 5
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 6
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 9
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 10
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 11
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 12
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 13
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 14
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 15
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 16
	// line 3
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 2
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 3
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 4
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 5
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 6
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 9
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 10
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 11
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 12
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 13
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 14
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 15
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 16
	// line 4
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 2
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 3
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 4
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 5
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 6
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 9
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 10
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 11
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 12
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 13
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 14
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 15
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // 16
}};

/************************** Variable Definitions *****************************/
unsigned char u8Verbose = 0;
unsigned char u8UserIO = 0;

// Driver instance variables
static XUartNs550 sUartInst;
static XVtc sVtcInst;
static XGpio sVidPathGpio;
static XTpg sTpgInst;
static XSysMon sXADC;
static XGpio sUserIO;
static XIntc sIntc;
static XGpio sOledGpio;
static XSpi sOledSpi;
static XIic sIic;

// interrupt vector table
const ivt_t ivt[] = {
	// User I/O
	{XPAR_MICROBLAZE_0_AXI_INTC_USERIO_IP2INTC_IRPT_INTR, (XInterruptHandler)fnUserIOIsr, &sUserIO},
	//IIC
	{XPAR_INTC_0_IIC_0_VEC_ID, (XInterruptHandler)XIic_InterruptHandler, &sIic},
	//OLED
	{XPAR_INTC_0_SPI_1_VEC_ID, (XInterruptHandler)XSpi_InterruptHandler, &sOledSpi}
};

struct netif server_netif;
struct netif *echo_netif;
volatile macAddress_t mac;
volatile XStatus macStatus;

/************************** Function Prototypes ******************************/
cmds_t fnUartCmdDecode(const char *szCmd);
void Asserted (const char *File, int Line);


/************************** Function Definitions *****************************/
XStatus main() {

	unsigned char eBistCurrentStatus = STATUS_BIST_NOT_STARTED_IDLE;
	cmds_t eBistCurrentCommand = INVALID;
	unsigned short iCmdChar = 0;
	char chUartMsg[MAX_BIST_CMD_LENGTH];

	XStatus Status;
	u32 u32CntOledCurtain = 0;
	u8 fOledCurtain = 0;
	int PhyTestStatus = 0;

	echo_netif = &server_netif;

	Xil_AssertSetCallback(Asserted);

	// Enabling caches
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheEnable();
#endif


	// Initialize UART
	Status = fnUartInit(&sUartInst);
	if(Status != XST_SUCCESS) {
		xil_printf("\r\nError initializing UART");
		return XST_FAILURE;
	}

	// Initialize the interrupt controller
	Status = fnInitInterruptController(&sIntc);
	if(Status != XST_SUCCESS) {
		xil_printf("\r\nError initializing interrupts");
		return XST_FAILURE;
	}

	// Initialize IIC controller
	Status = fnInitIic(&sIic);
	if(Status != XST_SUCCESS) {
		xil_printf("Error initializing I2C controller");
		return XST_FAILURE;
	}

	// Initialize XADC
	Status = fnXADCInit(&sXADC);
	if(Status != XST_SUCCESS) {
		xil_printf("\r\nError initializing XADC");
		return XST_FAILURE;
	}

	// Initialize VTC
	Status = fnVtcInit(&sVtcInst);
	if(Status != XST_SUCCESS) {
		xil_printf("\r\nError initializing VTC");
		return XST_FAILURE;
	}

	// Initialize TPG
	Status = fnTpgInit(&sTpgInst);
	if(Status != XST_SUCCESS) {
		xil_printf("\r\nError initializing TPG");
		return XST_FAILURE;
	}

	// Enable mouse pointer
	fnPS2MouseShow(TRUE);

	// Initialize GPIO controlling the video muxes
	Status = fnInitVidMux(&sVidPathGpio);
	if(Status != XST_SUCCESS) {
		xil_printf("\r\nError initializing Video Mux");
		return XST_FAILURE;
	}

	//Initialize Audio
	Status = fnInitAudio();
	if(Status != XST_SUCCESS) {
		xil_printf("\r\nError initializing Audio");
		return XST_FAILURE;
	}

	// Initializing User I/O GPIO driver
	Status = fnInitUserIO(&sUserIO);
	if(Status != XST_SUCCESS) {
		xil_printf("\r\nError initializing User I/O");
		return XST_FAILURE;
	}

	// Initialize the OLED driver
	Status = fnOledDriverInit(&sOledSpi, &sOledGpio);
	if(Status != XST_SUCCESS) {
		xil_printf("\r\nError during OLED driver initialization");
		return XST_FAILURE;
	}

	// Enable all interrupts in our interrupt vector table
	// Make sure all driver instances using this IVT are initialized first
	fnEnableInterrupts(&sIntc, &ivt[0], sizeof(ivt)/sizeof(ivt[0]));

	fnReadMACAsync(&sIic, &mac, &macStatus);

	// Test the Ethernet Phy
	// Wait for about 2 S until the autonegotiation of the phy completes, if any
	MB_Sleep(5000);
	PhyTestStatus = fnPhyTest();

	if((PhyTestStatus == 1000) || (PhyTestStatus == 100)) {
		// Initialize Ethernet and DHCP if there is a link
		while (macStatus == XST_DEVICE_BUSY) ; // Wait for MAC address to be read
		if (macStatus != XST_SUCCESS)
		{
			xil_printf("\r\nError during MAC address read");
			return XST_FAILURE;
		}

		Status = fnInitEth(&sIntc, &mac);
		if(Status != XST_SUCCESS) {
			xil_printf("\r\nError initializing Ethernet");
			return XST_FAILURE;
		}
	}

	// Setup OLED driver options after interrupts are initialized
	Status = fnOledDriverOptions(&sOledSpi);
	if(Status != XST_SUCCESS) {
		xil_printf("\r\nError during OLED driver options setup");
		return XST_FAILURE;
	}
	else {
		Status = fnOledDisplayInit(&sOledSpi, &sOledGpio);
		if(Status != XST_SUCCESS) {
			xil_printf("\r\nError during OLED display initialization");
			return XST_FAILURE;
		}
	}

	//xil_printf("\033[H\033[J"); //Clear the terminal
	xil_printf("----------------------------------------------------------\r\n");
	xil_printf("Nexys-VIDEO Rev. C Board Manufacturing Test Image 2\r\n");
	xil_printf("----------------------------------------------------------\r\n");

	// XXX: What is this for?
	xil_printf("RDY\n");

	while(1) {

		// Switch the curtain
		if(u32CntOledCurtain >= 3000000) {
			fnOledPixelToDisplay(&sOledSpi, &sOledGpio, fOledCurtain ?
					&sLeftCurtain : &sRightCurtain);
			u32CntOledCurtain = 0;
			fOledCurtain = !fOledCurtain;
		}

		// Check if a command was received from the UART
		if(XUartNs550_IsReceiveData(sUartInst.BaseAddress)) {
			chUartMsg[iCmdChar] = XUartNs550_RecvByte(sUartInst.BaseAddress);
			if(!((chUartMsg[iCmdChar] >= 'a' && chUartMsg[iCmdChar] <= 'z') ||
				 (chUartMsg[iCmdChar] >= 'A' && chUartMsg[iCmdChar] <= 'Z') ||
				 (chUartMsg[iCmdChar] >= '0' && chUartMsg[iCmdChar] <= '9') ||
				 chUartMsg[iCmdChar] == '-')){
				// Termination character
				chUartMsg[iCmdChar] = '\0';
				eBistCurrentCommand = fnUartCmdDecode((const char *)chUartMsg);
				eBistCurrentStatus = STATUS_COMMAND_RECEIVED;
				iCmdChar = 0;
			}
			else {
				iCmdChar++;
				if(!(iCmdChar < MAX_BIST_CMD_LENGTH)) {
					iCmdChar = 0;
				}
			}
		}

		// The main test selector based on the decoded command
		if(eBistCurrentStatus == STATUS_COMMAND_RECEIVED) {
			switch(eBistCurrentCommand) {
			case TEST_UART:
				Status = fnUartTest(&sUartInst);
				if(Status != XST_SUCCESS) {
					xil_printf("FAIL");
				}
				else {
					xil_printf("PASS");
				}
				eBistCurrentStatus = STATUS_BIST_STARTED_IDLE;
				break;
			case TEST_QSPI:
				eBistCurrentStatus = STATUS_BIST_STARTED_IDLE;
				break;
			case TEST_ETH_START:
				if ((PhyTestStatus != 1000) && (PhyTestStatus !=100)){
					xil_printf("FAIL\n\r");
				}
				else{
					xil_printf("\r\nLink Speed = %d\n\r", PhyTestStatus);
					fnPrintIpSettings();
					while(1) {
						xemacif_input(echo_netif);
						transfer_data();
						// Check if the "eth-stop command was received from the UART
						if(XUartNs550_IsReceiveData(sUartInst.BaseAddress)) {
							chUartMsg[iCmdChar] = XUartNs550_RecvByte(sUartInst.BaseAddress);
							if(!((chUartMsg[iCmdChar] >= 'a' && chUartMsg[iCmdChar] <= 'z') ||
								 (chUartMsg[iCmdChar] >= 'A' && chUartMsg[iCmdChar] <= 'Z') ||
								 (chUartMsg[iCmdChar] >= '0' && chUartMsg[iCmdChar] <= '9') ||
								 chUartMsg[iCmdChar] == '-')) {
								// Termination character
								chUartMsg[iCmdChar] = '\0';
								eBistCurrentCommand = fnUartCmdDecode((const char *)chUartMsg);
								eBistCurrentStatus = STATUS_COMMAND_RECEIVED;
								iCmdChar = 0;
							}
							else {
								iCmdChar++;
								if(!(iCmdChar < MAX_BIST_CMD_LENGTH)) {
									iCmdChar = 0;
								}
							}
						}
						if((eBistCurrentStatus == STATUS_COMMAND_RECEIVED)
							&& (eBistCurrentCommand == TEST_ETH_STOP)) {
								break; //exit the while loop
						}
					}
				}
				eBistCurrentStatus = STATUS_BIST_STARTED_IDLE;
				break;
			case TEST_DP:
				// Set the generated pattern to the DVI output
				fnSwitchDVItoPattern(&sVidPathGpio);
				// Loop-back video data from DVI input to the DisplayPort output
				fnSwitchDPtoDVI(&sVidPathGpio);
				// Call the Policy Maker
				Status = fnDpTest(&sVtcInst);
				if(Status != XST_SUCCESS) {
					xil_printf("FAIL\n");
				}
				else {
					xil_printf("PASS\n");
				}
				eBistCurrentStatus = STATUS_BIST_STARTED_IDLE;
				break;
			case TEST_SD:
				Status = fnSdTest();
				if(Status != XST_SUCCESS) {
					xil_printf("FAIL\n");
				}
				else {
					xil_printf("PASS\n");
				}
				eBistCurrentStatus = STATUS_BIST_STARTED_IDLE;
				break;
			case TEST_AUDIO:
				fnSetLineInput();
				fnAudioRecord(192000);
				fnSetLineOutput();
				fnSetMicInput();
				fnAudioPlayAndRecord(192000);
				fnSetHpOutput();
				fnAudioPlay(192000);
				xil_printf("Test audio Lin -> Lout -> MIC -> Headphones finished\n");
				eBistCurrentStatus = STATUS_BIST_STARTED_IDLE;
				break;
			case TEST_USERIO_START:
				xil_printf("User I/O ON\n");
				u8UserIO = 1;
				eBistCurrentStatus = STATUS_BIST_STARTED_IDLE;
				break;
			case TEST_USERIO_STOP:
				xil_printf("User I/O OFF\n");
				u8UserIO = 0;
				eBistCurrentStatus = STATUS_BIST_STARTED_IDLE;
				break;
			case VERBOSE_ON:
				xil_printf("Verbose ON\n");
				u8Verbose = 1;
				eBistCurrentStatus = STATUS_BIST_STARTED_IDLE;
				break;
			case VERBOSE_OFF:
				xil_printf("Verbose OFF\n");
				u8Verbose = 0;
				eBistCurrentStatus = STATUS_BIST_STARTED_IDLE;
				break;
			default:
				eBistCurrentStatus = STATUS_BIST_STARTED_IDLE;
				break;
			}
		}

		// Increment the OLED
		u32CntOledCurtain++;
	}

	return XST_SUCCESS;
}

/******************************************************************************
 * Function that decodes the current UART-received command and returns the
 * corresponding command type.
 *
 * @param	szCmd is a pointer to the UART-received command.
 *
 * @return	command type.
 *
 *****************************************************************************/
cmds_t fnUartCmdDecode(const char *szCmd) {

	int iCmd = sizeof(bistCommands)/sizeof(bistCommands[0]) - 1;

	// Jump-table-like stuff
	for(; iCmd >= 0; --iCmd) {
		if(!strcmp(szCmd, bistCommands[iCmd].pchCmd)) {
			return bistCommands[iCmd].eCmd;
		}
	}

	return INVALID;
}

void Asserted (const char *File, int Line) {

}

