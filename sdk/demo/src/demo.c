/******************************************************************************
 * @file demo.c
 * This is the 'Out of Box' demo application for the Digilent Nexys Video board.
 * It runs as a standalone app on an embedded Microblaze system.
 *
 * @authors Elod Gyorgy, Mihaita Nagy
 *
 * @date 2015-Jan-21
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
 * User I/O Demo:	This demo lights up the LED(s) above the active switch(es)
 * 					and by pressing any of the push-buttons (BTNU, BTNL, BTNC,
 * 					BTNR, BTND) the state of the LEDs is inverted.
 *
 * Audio Demo:		The audio demo records a 5 second sample from microphone
 * 					(J6) or line in (J7) and plays it back on headphone out
 * 					(J4) or line out (J5). Record and playback is started by
 * 					pushbuttons:
 * 					BTNU: record from microphone
 * 					BTNR: record from line in
 * 					BTND: playback on headphone
 * 					BTNL: playback on line out
 * 					For example, with the push of BTNU this demo records 5
 * 					seconds of audio data from the MIC (J6) input. Consequently
 * 					by pressing BTND the demo plays on the HPH OUT (J4) the
 * 					recorded samples.
 *
 * Ethernet Demo:	The demo uses the lwIP stack to implement an pingable echo
 * 					server. The on-board EEPROM is read to find out the MAC
 * 					address of the board. The network interface initializes as
 * 					soon as a cable is plugged in and link is detected. The DHCP
 * 					client will	try to get an IP, which will be shown on both the
 * 					on-board OLED (DISP1) and the terminal.
 * 					Once the IP address is displayed, the server replies to ping
 * 					requests on port 7.
 *
 * Video Demo: 		With a DVI source connected to J9 (HDMI IN) the demo works
 *					as a pass-through buffer outputting the video data on DVI
 *					output J8 (HDMI OUT) and also J10 (DISPLAYPORT OUT).
 * 					In the absence of a DVI source, the demo automatically
 * 					switches the outputs (DVI and DisplayPort) to an internally
 * 					generated video pattern.
 *
 * XADC Demo:		The XADC is set up to monitor the internal FPGA temperature,
 * 					VCCINT voltage and VCCAUX voltage. The XADC is periodically
 * 					read and the readings are displayed on the OLED and the
 * 					terminal.
 *
 * OLED Demo:		The on-board OLED shows the Digilent logo on power-up. After
 * 					a two-second timeout it switches to display the XADC readings
 * 					and the local IP address the Ethernet interface acquired.
 * 					The OLED driver uses the GPIO and SPI IPs to talk to the
 * 					display.
 *
 * UART Demo:		Status messages and XADC readings are shown on the terminal.
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
 * 1.01  Elod Gyorgy  2015-Jan-21 Warm-reset fix.
 * 1.00  Mihaita Nagy 2015-Jan-15 First release
 *  	 Elod Gyorgy
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "demo.h"
#include <string.h>
#include "xuartns550_l.h"

#include "audio/audio.h"
#include "dma/dma.h"
#include "dp/dp.h"
#include "intc/intc.h"
#include "oled/oled.h"
#include "userio/userio.h"
#include "video/video.h"
#include "xadc/xadc.h"
#include "iic/iic.h"
#include "eth/eth.h"

/************************** Variable Definitions *****************************/
static XGpio sUserIO;
static XIntc sIntc;
static XGpio sVidPathGpio;
static XTpg sTpgInst;
static XVtc sVtdInst;
static XSysMon sXADC;
static XIic sIic;
static XGpio sOledGpio;
static XSpi sOledSpi;
static XAxiDma sAxiDma;


// This variable holds the demo related settings
volatile sDemo_t Demo;

/************************** Constant Definitions *****************************/
// UART baud rate
#define UART_BAUD 				115200

// Audio constants
// Number of seconds to record/playback
#define NR_SEC_TO_REC_PLAY		5

// ADC/DAC sampling rate in Hz
#define AUDIO_SAMPLING_RATE		96000

// Number of samples to record/playback
#define NR_AUDIO_SAMPLES		(NR_SEC_TO_REC_PLAY*AUDIO_SAMPLING_RATE)

// Interrupt vector table
const ivt_t ivt[] = {
	//User I/O (buttons, switches, LEDs)
	{XPAR_MICROBLAZE_0_AXI_INTC_USERIO_IP2INTC_IRPT_INTR, (XInterruptHandler)fnUserIOIsr, &sUserIO},
	//XADC Not using interrupts
  	//{XPAR_INTC_0_SYSMON_0_VEC_ID, (XInterruptHandler)fnXADCIsr, &sXADC},
	//ETH_INT_B PHY interrupt
	{XPAR_MICROBLAZE_0_AXI_INTC_SYSTEM_ETH_INT_B_0_INTR, (XInterruptHandler)fnEthPHYIsr, NULL},
	//IIC
	{XPAR_MICROBLAZE_0_AXI_INTC_AXI_IIC_0_IIC2INTC_IRPT_INTR, (XInterruptHandler)XIic_InterruptHandler, &sIic},
	//Video Path (DVI Input Clock locked detection)
	{XPAR_MICROBLAZE_0_AXI_INTC_VIDEO_VIDEO_PATH_CTRL_IP2INTC_IRPT_INTR, (XInterruptHandler)fnVideoPathCtrlIsr, &sVidPathGpio},
	//OLED SPI Interrupt handler
	{XPAR_MICROBLAZE_0_AXI_INTC_OLED_AXI_QUAD_SPI_0_IP2INTC_IRPT_INTR, (XInterruptHandler)XSpi_InterruptHandler, &sOledSpi},
	//DMA Stream to MemoryMap Interrupt handler
	{XPAR_MICROBLAZE_0_AXI_INTC_AXI_DMA_0_S2MM_INTROUT_INTR, (XInterruptHandler)fnS2MMInterruptHandler, &sAxiDma},
	//DMA MemoryMap to Stream Interrupt handler
	{XPAR_MICROBLAZE_0_AXI_INTC_AXI_DMA_0_MM2S_INTROUT_INTR, (XInterruptHandler)fnMM2SInterruptHandler, &sAxiDma}
};

/************************** Function Prototypes ******************************/
void Asserted (const char *File, int Line);

/************************** Function Definitions *****************************/
XStatus main() {

	XStatus Status;
	char ScreenBuf[64];

	// Enabling caches
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheEnable();
#endif

	// Empty assert function. We can put a breakpoint there to look for asserts
	Xil_AssertSetCallback(Asserted);

    // Initialize UART
    XUartNs550_SetBaud(XPAR_AXI_UART16550_0_BASEADDR,
    		XPAR_XUARTNS550_CLOCK_HZ, UART_BAUD);
    XUartNs550_SetLineControlReg(XPAR_AXI_UART16550_0_BASEADDR,
    		XUN_LCR_8_DATA_BITS);

    xil_printf("\033[H\033[J"); //Clear the terminal
    xil_printf("\r\nInitializing demo: ");

	// Initialize the interrupt controller
	Status = fnInitInterruptController(&sIntc);
	if(Status != XST_SUCCESS) {
		xil_printf("Error initializing interrupts");
		return XST_FAILURE;
	}

	// Initialize the OLED driver
	Status = fnOledDriverInit(&sOledSpi, &sOledGpio);
	if(Status != XST_SUCCESS) {
		xil_printf("OLED driver initialization ERROR");
		return XST_FAILURE;
	}

	// Initialize IIC controller
	Status = fnInitIic(&sIic);
	if(Status != XST_SUCCESS) {
		xil_printf("Error initializing I2C controller");
		return XST_FAILURE;
	}

    // Initialize User I/O driver
    Status = fnInitUserIO(&sUserIO);
    if(Status != XST_SUCCESS) {
    	xil_printf("User I/O ERROR");
    	return XST_FAILURE;
    }

	// Initialize VTD
	Status = fnVtdInit(&sVtdInst);
	if(Status != XST_SUCCESS) {
		xil_printf("VTD initialization ERROR");
		return XST_FAILURE;
	}

	// Initialize TPG
	Status = fnTpgInit(&sTpgInst);
	if(Status != XST_SUCCESS) {
		xil_printf("TPG initialization ERROR");
		return XST_FAILURE;
	}

	// Initialize GPIO controlling the video muxes
	Status = fnInitVidMux(&sVidPathGpio);
	if(Status != XST_SUCCESS) {
		xil_printf("Video Mux initialization ERROR");
		return XST_FAILURE;
	}

	// Initialize XADC
	Status = fnXADCInit(&sXADC);
	if(Status != XST_SUCCESS) {
		xil_printf("XADC initialization ERROR");
		return XST_FAILURE;
	}

	//Initialize DMA
	Status = fnConfigDma(&sAxiDma);
	if(Status != XST_SUCCESS) {
		xil_printf("DMA configuration ERROR");
		return XST_FAILURE;
	}

	//Initialize Audio I2S
	Status = fnInitAudio();
	if(Status != XST_SUCCESS) {
		xil_printf("Audio initializing ERROR");
		return XST_FAILURE;
	}

	//Initialize Ethernet PHY
	Status = fnPhyInit();
	if(Status != XST_SUCCESS) {
		xil_printf("Ethernet PHY initialization ERROR");
		return XST_FAILURE;
	}

	// Enable all interrupts in our interrupt vector table
	// Make sure all driver instances using interrupts are initialized first
	fnEnableInterrupts(&sIntc, &ivt[0], sizeof(ivt)/sizeof(ivt[0]));

	// Setup OLED driver options after interrupts are initialized
	Status = fnOledDriverOptions(&sOledSpi);
	if(Status != XST_SUCCESS) {
		xil_printf("OLED driver options setup ERROR");
		return XST_FAILURE;
	}
	else {
		Status = fnOledDisplayInit(&sOledSpi, &sOledGpio);
		if(Status != XST_SUCCESS) {
			xil_printf("OLED display initialization ERROR");
			return XST_FAILURE;
		}
	}

	// Read MAC with interrupts
	fnReadMACAsync(&sIic, &Demo.mac, &Demo.fMacStatus);

	// Enable mouse pointer
	fnPS2MouseShow(TRUE);

    xil_printf("DONE\r\n");

    MB_Sleep(2000);

	int temp = 0, vccint = 0, vccaux = 0;

    while(1) {

    	u32 dwXADCStatus;

    	xil_printf("\033[H\033[J"); //Clear the terminal
    	xil_printf("----------------------------------------------------------\r\n");
		xil_printf("Nexys Video demo application\r\n");
		xil_printf("----------------------------------------------------------\r\n");

		if ((dwXADCStatus = XSysMon_GetStatus(&sXADC)) & XSM_SR_EOS_MASK) {
			// If XADC reads 0, ignore it
			int read = (int)XSysMon_RawToTemperature(XSysMon_GetAdcData(&sXADC, XSM_CH_TEMP));
			if (read != -273) temp = read;
			read = (int)(XSysMon_RawToVoltage(XSysMon_GetAdcData(&sXADC, XSM_CH_VCCINT)) * 1000);
			if (read != 0) vccint = read;
			read = (int)(XSysMon_RawToVoltage(XSysMon_GetAdcData(&sXADC, XSM_CH_VCCAUX)) * 1000);
			if (read != 0) vccaux = read;

			// Formating the XADC data for the OLED display
    		sprintf(&ScreenBuf[0],  "FPGA temp:%4d C", temp);
    		sprintf(&ScreenBuf[OLED_MAX_LINE_LEN], "VCCINT:  %4d mV", vccint);
    		sprintf(&ScreenBuf[OLED_MAX_LINE_LEN*2], "VCCAUX:  %4d mV", vccaux);
    		sprint_ip(&ScreenBuf[OLED_MAX_LINE_LEN*3]);
    		fnOledStringToDisplay(&sOledSpi, &sOledGpio, &ScreenBuf[0]);

			xil_printf("\r\nInternal temperature: %d C", temp);
			xil_printf("\r\nVCCINT voltage: %d mV", vccint);
			xil_printf("\r\nVCCAUX voltage: %d mV", vccaux);
    	}

    	// Report Ethernet link status
    	xil_printf("\r\nEthernet Link ");
    	if (Demo.fLinkStatus) {
    		xil_printf("up. Speed %dMbps. IP ", Demo.linkSpeed); print_echo_ip();
		}
    	else {
    		xil_printf("down.");
    	}

		if(Demo.fDVIClockLock) {
			xil_printf("\r\nShowing DVI input.");
		}
		else {
			xil_printf("\r\nShowing video test pattern.");
		}

    	// Checking the video event flag
		if(Demo.fVideoEvent) {

			// Run the Policy Maker
			Status = fnDpTest(&sVtdInst);
			if(Status == XST_NO_DATA) {
				xil_printf("\r\nDisplayport timeout!");
			}

			// Reset the video event flag
			Demo.fVideoEvent = 0;
		}

		// Checking the DMA S2MM event flag
		if (Demo.fDmaS2MMEvent)
		{
			xil_printf("\r\nRecording Done...");

			// Disable Stream function to send data (S2MM)
			Xil_Out32(I2S_STREAM_CONTROL_REG, 0x00000000);
			Xil_Out32(I2S_TRANSFER_CONTROL_REG, 0x00000000);
			//Flush cache
			microblaze_flush_dcache();
			microblaze_invalidate_dcache();
			// Reset S2MM event and record flag
			Demo.fDmaS2MMEvent = 0;
			Demo.fAudioRecord = 0;
		}

		// Checking the DMA MM2S event flag
		if (Demo.fDmaMM2SEvent)
		{
			xil_printf("\r\nPlayback Done...");

			// Disable Stream function to send data (S2MM)
			Xil_Out32(I2S_STREAM_CONTROL_REG, 0x00000000);
			Xil_Out32(I2S_TRANSFER_CONTROL_REG, 0x00000000);
			//Flush cache
			microblaze_flush_dcache();
			microblaze_invalidate_dcache();
			// Reset MM2S event and playback flag
			Demo.fDmaMM2SEvent = 0;
			Demo.fAudioPlayback = 0;
		}

		// Checking the DMA Error event flag
		if (Demo.fDmaError)
		{
			xil_printf("\r\nDma Error...");
			xil_printf("\r\nDma Reset...");
			Demo.fDmaError = 0;
			Demo.fAudioPlayback = 0;
			Demo.fAudioRecord = 0;
		}

		// Checking the btn change event
		if(Demo.fUserIOEvent) {

			switch(Demo.chBtn) {
				case 'u':
					if (!Demo.fAudioRecord && !Demo.fAudioPlayback)
					{
						xil_printf("\r\nStart Recording...");
						fnSetMicInput();
						fnAudioRecord(sAxiDma,NR_AUDIO_SAMPLES);
						Demo.fAudioRecord = 1;
					}
					else
					{
						if (Demo.fAudioRecord)
						{
							xil_printf("\r\nStill Recording...");
						}
						else
						{
							xil_printf("\r\nStill Playing back...");
						}
					}
					break;
				case 'd':
					if (!Demo.fAudioRecord && !Demo.fAudioPlayback)
					{
						xil_printf("\r\nStart Playback...");
						fnSetHpOutput();
						fnAudioPlay(sAxiDma,NR_AUDIO_SAMPLES);
						Demo.fAudioPlayback = 1;
					}
					else
					{
						if (Demo.fAudioRecord)
						{
							xil_printf("\r\nStill Recording...");
						}
						else
						{
							xil_printf("\r\nStill Playing back...");
						}
					}
					break;
				case 'r':
					if (!Demo.fAudioRecord && !Demo.fAudioPlayback)
					{
						xil_printf("\r\nStart Recording...");
						fnSetLineInput();
						fnAudioRecord(sAxiDma,NR_AUDIO_SAMPLES);
						Demo.fAudioRecord = 1;
					}
					else
					{
						if (Demo.fAudioRecord)
						{
							xil_printf("\r\nStill Recording...");
						}
						else
						{
							xil_printf("\r\nStill Playing back...");
						}
					}
					break;
				case 'l':
					if (!Demo.fAudioRecord && !Demo.fAudioPlayback)
					{
						xil_printf("\r\nStart Playback...");
						fnSetLineOutput();
						fnAudioPlay(sAxiDma,NR_AUDIO_SAMPLES);
						Demo.fAudioPlayback = 1;
					}
					else
					{
						if (Demo.fAudioRecord)
						{
							xil_printf("\r\nStill Recording...");
						}
						else
						{
							xil_printf("\r\nStill Playing back...");
						}
					}
					break;
				default:
					break;
			}

			// Reset the user I/O flag
			Demo.chBtn = 0;
			Demo.fUserIOEvent = 0;
		}

		if (Demo.fLinkEvent) {

			if (Demo.fLinkStatus) {
				//If we acquired link, try to initialize interface
				// The MAC address should have been read by now
				if (Demo.fMacStatus == XST_SUCCESS) {
					fnStartEth(&sIntc, &Demo.mac);
				}
				else
					xil_printf("\r\nError reading Ethernet MAC from EEPROM");
			}
			else {
				fnCloseEth();
			}
			Demo.fLinkEvent = 0;
		}

    	fflush(stdout);
    	MB_Sleep(1000);
    }

    // Disabling caches
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_ICacheDisable();
#endif

    return XST_SUCCESS;
}

void Asserted (const char *File, int Line) {
	//We could write something here to catch asserts
}


