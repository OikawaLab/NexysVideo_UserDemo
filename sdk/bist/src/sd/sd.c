/******************************************************************************
 * @file sd.c
 *
 * This file contains the initialization state machine of the SD in Native mode.
 * It does not control any transactions nor it initiates any but it reads the
 * Extended Identification and calculates CRC on every wire comparing them with
 * the actual read ones.
 *
 * @authors Mihaita Nagy
 *
 * @date 2014-Sep-09
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
 * 1.00  Mihaita Nagy 2014-Sep-09 First release
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "sd.h"

/************************** Constant Definitions *****************************/
#define SD_BASEADDRESS		XPAR_D_SD_DEMO_0_S00_AXI_BASEADDR
#define CD_BASEADDRESS		XPAR_SD_GPIO_BASEADDR

#define SCK_EN_MASK			0xfffffffe
#define SCK_DIV_MASK		0xfffffff1
#define RESP_TYPE_MASK		0xffffffcf
#define DATA_TRANSFER_MASK	0xffffff7f
#define CMD_INDEX_MASK		0xffff00ff

typedef struct sRegs {
	u32 command_reg;
	u32 argument_reg;
	u32 status_reg;
	u32 response_reg0;
	u32 response_reg1;
	u32 response_reg2;
	u32 response_reg3;
	u32 dbg_reg0;
	u32 dbg_reg1;
	u32 dbg_reg2;
	u32 dbg_reg3;
} SdRegs_t;

SdRegs_t reg = {
	SD_BASEADDRESS,
	SD_BASEADDRESS + 0x04,
	SD_BASEADDRESS + 0x08,
	SD_BASEADDRESS + 0x0c,
	SD_BASEADDRESS + 0x10,
	SD_BASEADDRESS + 0x14,
	SD_BASEADDRESS + 0x18,
	SD_BASEADDRESS + 0x1c,
	SD_BASEADDRESS + 0x20,
	SD_BASEADDRESS + 0x24,
	SD_BASEADDRESS + 0x28
};

enum eCommandReg {
	sck_en 			= 0,
	sck_div 		= 1,
	resp_type		= 4,
	data_transfer 	= 7,
	cmd_index 		= 8,
};

enum eDivRate {
	div_2			= 0, // 000 --> 50 MHz
	div_4			= 1, // 001 --> 25 MHz
	div_10			= 2, // 010 --> 10 MHz
	div_20			= 3, // 011 -->  5 MHz
	div_50			= 4, // 100 -->  2 MHz
	div_100			= 5, // 101 -->  1 MHz
	div_125			= 6, // 110 --> 800 kHz
	div_250			= 7  // 111 --> 200 kHz
};

enum eStatusReg {
	cmd_done 		= 0,
	resp_done 		= 4,
	resp_timeout	= 6,
	data_done		= 8,
	zero_data0		= 10,
	zero_data1		= 11,
	zero_data2		= 12,
	zero_data3		= 13,
	error_data0		= 14,
	error_data1		= 15,
	error_data2		= 16,
	error_data3		= 17
};

enum eCommands {
	CMD0			= 0x40,
	CMD2			= 0x42,
	CMD3			= 0x43,
	CMD7			= 0x47,
	CMD8			= 0x48,
	CMD9			= 0x49,
	CMD16			= 0x50,
	CMD19			= 0x53,
	CMD55			= 0x77,
	CMD58			= 0x7A,
	ACMD6			= 0x46,
	ACMD13			= 0x4D,
	ACMD41			= 0x69
};

/************************** Variable Definitions *****************************/
extern unsigned char u8Verbose;

/************************** Function Definitions *****************************/

/******************************************************************************
 * Function that calls the SD initialization function and read the Extended ID
 * in Native mode (all 4 wires) and does CRC on each wire.
 *
 * @param	none.
 *
 * @note	See SD Native standard 1.1
 *
 * @return	XST_SUCCESS - on success
 * 			XST_FAILURE - on failure
 *****************************************************************************/
XStatus fnSdTest() {

	u16 u16Ret, u16Rca;
	u8 usz8ErrorData[4], usz8ZeroData[4];

	if(u8Verbose) {
		xil_printf("\r\nInsert card...");
	}

	if (Xil_In32(CD_BASEADDRESS))
	{
		if(u8Verbose) {
			xil_printf("No card detected.");
		}
		return XST_FAILURE;
	}

	if(u8Verbose) {
		xil_printf(" OK.");
	}

	MB_Sleep(10);

	if(u8Verbose) {
		xil_printf("\r\n\r\nStarted initialization...");
	}
	u16Ret = fnSdInit();
	if(u16Ret == 0xffff) {
		if(u8Verbose) {
			xil_printf("\r\nError initializing.\r\n");
		}
		return XST_FAILURE;
	}
	else {
		// storing RCA
		u16Rca = u16Ret;

		if(u8Verbose) {
			xil_printf("\r\nDone initializing.\r\n");
			xil_printf("\r\nReading SD Status on 4 bits...");
		}

		// card initialized and in state = 'stby'
		// to put it into 'tran' state we shall issue CMD7
		fnSdSendCmd(CMD7, ((u16Rca << 16) & 0xffff0000), 2, 0);
		if(u8Verbose) {
			xil_printf("\r\nCard is locked = %s",
					(((Xil_In32(reg.response_reg0) >> 25) & 0x00000001)==1)?
					"card locked":"card unlocked");
			xil_printf("\r\nCurrent state (CMD7) = %d",
					((Xil_In32(reg.response_reg0) >> 9) & 0x0000000f));
		}

		// sending ACMD6 to switch to 4-bit mode
		fnSdSendCmd(CMD55, ((u16Rca << 16) & 0xffff0000), 2, 0);
		if(u8Verbose) {
			xil_printf("\r\nCurrent state (CMD55) = %d",
					((Xil_In32(reg.response_reg0) >> 9) & 0x0000000f));
		}
		fnSdSendCmd(ACMD6, 0x00000002, 2, 0);
		if(u8Verbose) {
			xil_printf("\r\nCurrent state (ACMD6) = %d",
					((Xil_In32(reg.response_reg0) >> 9) & 0x0000000f));
		}

		//sending ACMD13 to read SD Status
		fnSdSendCmd(CMD55, ((u16Rca << 16) & 0xffff0000), 2, 0);
		if(u8Verbose) {
			xil_printf("\r\nCurrent state (CMD55) = %d",
					((Xil_In32(reg.response_reg0) >> 9) & 0x0000000f));
		}
		fnSdSendCmd(ACMD13, 0x00000000, 2, 1);
		if(u8Verbose) {
			xil_printf("\r\nCurrent state (ACMD13) = %d",
					((Xil_In32(reg.response_reg0) >> 9) & 0x0000000f));
		}

		usz8ZeroData[0] = (Xil_In32(reg.status_reg) >> zero_data0) & 0x00000001;
		usz8ZeroData[1] = (Xil_In32(reg.status_reg) >> zero_data1) & 0x00000001;
		usz8ZeroData[2] = (Xil_In32(reg.status_reg) >> zero_data2) & 0x00000001;
		usz8ZeroData[3] = (Xil_In32(reg.status_reg) >> zero_data3) & 0x00000001;
		usz8ErrorData[0] = (Xil_In32(reg.status_reg) >> error_data0) & 0x00000001;
		usz8ErrorData[1] = (Xil_In32(reg.status_reg) >> error_data0) & 0x00000001;
		usz8ErrorData[2] = (Xil_In32(reg.status_reg) >> error_data0) & 0x00000001;
		usz8ErrorData[3] = (Xil_In32(reg.status_reg) >> error_data0) & 0x00000001;

		if(u8Verbose) {
			xil_printf("\r\nCRC result:");
			xil_printf("\r\nDAT0: %s", (usz8ErrorData[0]==1)?"error":
					(usz8ZeroData[0]==1)?"all 0's":"OK");
			xil_printf("\r\nDAT1: %s", (usz8ErrorData[1]==1)?"error":
					(usz8ZeroData[1]==1)?"all 0's":"OK");
			xil_printf("\r\nDAT2: %s", (usz8ErrorData[2]==1)?"error":
					(usz8ZeroData[2]==1)?"all 0's":"OK");
			xil_printf("\r\nDAT3: %s", (usz8ErrorData[3]==1)?"error":
					(usz8ZeroData[3]==1)?"all 0's":"OK");
		}

		if(u8Verbose) {
			xil_printf("\r\nDAT0: actual = 0x%04x > decoded = 0x%04x",
					(Xil_In32(reg.dbg_reg0) >> 16) & 0x0000ffff,
					Xil_In32(reg.dbg_reg0) & 0x0000ffff);
			xil_printf("\r\nDAT1: actual = 0x%04x > decoded = 0x%04x",
					(Xil_In32(reg.dbg_reg1) >> 16) & 0x0000ffff,
					Xil_In32(reg.dbg_reg1) & 0x0000ffff);
			xil_printf("\r\nDAT2: actual = 0x%04x > decoded = 0x%04x",
					(Xil_In32(reg.dbg_reg2) >> 16) & 0x0000ffff,
					Xil_In32(reg.dbg_reg2) & 0x0000ffff);
			xil_printf("\r\nDAT3: actual = 0x%04x > decoded = 0x%04x",
					(Xil_In32(reg.dbg_reg3) >> 16) & 0x0000ffff,
					Xil_In32(reg.dbg_reg3) & 0x0000ffff);
		}

		if((usz8ZeroData[0] == 1) || (usz8ZeroData[1] == 1) ||
		   (usz8ZeroData[2] == 1) || (usz8ZeroData[3] == 1) ||
		   (usz8ErrorData[0] == 1) || (usz8ErrorData[1] == 1) ||
		   (usz8ErrorData[2] == 1) || (usz8ErrorData[3] == 1)) {
			return XST_FAILURE;
		}

		if(u8Verbose) {
			xil_printf("\r\nDone reading Status.");
		}

		// sending CMD7 again to put card into 'stby' state
		//sd_send_cmd(CMD7, ((u16Rca << 16) & 0xffff0000), 2, 0);
		//xil_printf("\r\nCurrent state (CMD7) = %d",
		// ((Xil_In32(reg.response_reg0) >> 9) & 0x0000000f));
	}

	return XST_SUCCESS;
}

/******************************************************************************
 * Function to initialize the SD. With verbose active, this function displays
 * SD card information (CID contents).
 *
 * @param	none.
 *
 * @note	See SD Native standard 1.1
 *
 * @return	RCA - on success
 * 			0xFFFF - on failure
 *****************************************************************************/
u16 fnSdInit() {

	u32 u32Ret, usz32Resp[4];
	u16 u16Rca, u16RetErr = 0;
	u8 usz8Month[12];

	// setting divide rate for sck
	Xil_Out32(reg.command_reg, (div_4/*div_250*/ << sck_div) |
			(Xil_In32(reg.command_reg) & SCK_DIV_MASK));
	// enabling sck
	Xil_Out32(reg.command_reg, (1 << sck_en) |
			(Xil_In32(reg.command_reg) & SCK_EN_MASK));
	// wait for 80 * 5us = 400us
	MB_Sleep(400);

	// sending CMD0
	fnSdSendCmd(CMD0, 0x00000000, 0, 0);

	// sending CMD8
	u32Ret = fnSdSendCmd(CMD8, 0x000001AA, 2, 0);

	if(u32Ret) {
		if(u8Verbose) {
			xil_printf("\r\nError: no response for CMD8 (R7).");
		}
		u16RetErr = 0xffff;
	}
	else { // Ver2.00 or later SD Memory Card

		if(Xil_In32(reg.response_reg0) != 0x000001AA) {
			if(u8Verbose) {
				xil_printf("\r\nError: bad response for CMD8 (R7).");
			}
			u16RetErr = 0xffff;
		}
		else { // Card with compatible voltage range

			// sending ACMD41
			do {
				// enable ACMD by sending CMD55 with R1 in response
				u32Ret = fnSdSendCmd(CMD55, 0x00000000, 2, 0);
				if(u32Ret) {
					if(u8Verbose) {
						xil_printf("\r\nError: no response for CMD55 (R1).");
					}
					u16RetErr = 0xffff;
				}
				u32Ret = fnSdSendCmd(ACMD41, 0x40300000, 2, 0);
				if(u32Ret) {
					if(u8Verbose) {
						xil_printf("\r\nError: no response for ACMD41 (R3).");
					}
					u16RetErr = 0xffff;
				}
				usz32Resp[0] = Xil_In32(reg.response_reg0);
			} // wait for the busy bit in R3 to clear
			while(!((usz32Resp[0] >> 31) & 0x01));

			if(u8Verbose) {
				// checking CCS bit
				if((usz32Resp[0] >> 30) & 0x01) { // CCS = 1
					xil_printf("\r\nVer2.00 or later SDHC/SDXC Card.");
				}
				else { // CCS = 0
					xil_printf("\r\nVer2.00 or later SDSC.");
				}
			}

			// sending CMD2
			u32Ret = fnSdSendCmd(CMD2, 0x00000000, 1, 0);
			if(u32Ret) {
				if(u8Verbose) {
					xil_printf("\r\nError: no response for CMD2 (R2).");
				}
				u16RetErr = 0xffff;
			}

			// display CID contents
			if(u8Verbose) {
				xil_printf("\r\n\r\nCID:");
				usz32Resp[0] = Xil_In32(reg.response_reg0);
				usz32Resp[1] = Xil_In32(reg.response_reg1);
				usz32Resp[2] = Xil_In32(reg.response_reg2);
				usz32Resp[3] = Xil_In32(reg.response_reg3);

				switch(usz32Resp[0] & 0x0000000f) {
					case 1:
						strcpy((char *)usz8Month, "January");
						break;
					case 2:
						strcpy((char *)usz8Month, "February");
						break;
					case 3:
						strcpy((char *)usz8Month, "March");
						break;
					case 4:
						strcpy((char *)usz8Month, "April");
						break;
					case 5:
						strcpy((char *)usz8Month, "May");
						break;
					case 6:
						strcpy((char *)usz8Month, "June");
						break;
					case 7:
						strcpy((char *)usz8Month, "July");
						break;
					case 8:
						strcpy((char *)usz8Month, "August");
						break;
					case 9:
						strcpy((char *)usz8Month, "September");
						break;
					case 10:
						strcpy((char *)usz8Month, "October");
						break;
					case 11:
						strcpy((char *)usz8Month, "November");
						break;
					default:
						strcpy((char *)usz8Month, "December");
						break;
				}

				xil_printf("\r\nManufacturer ID             : 0x%x",
						(usz32Resp[3] >> 16) & 0x000000ff);
				xil_printf("\r\nOEM/Application ID          : %c%c",
						(usz32Resp[3] >> 8) & 0x000000ff,
						usz32Resp[3] & 0x000000ff);
				xil_printf("\r\nProduct name                : %c%c%c%c%c",
						(usz32Resp[2] >> 24) & 0x000000ff,
						(usz32Resp[2] >> 16) & 0x000000ff,
						(usz32Resp[2] >> 8) & 0x000000ff,
						usz32Resp[2] & 0x000000ff,
						(usz32Resp[1] >> 24) & 0x000000ff);
				xil_printf("\r\nProduct revision            : %d.%d",
						(usz32Resp[1] >> 20) & 0x0000000f,
						(usz32Resp[1] >> 16) & 0x0000000f);
				xil_printf("\r\nProduct serial number       : 0x%x",
						((usz32Resp[1] << 15) & 0xffff0000) |
						((usz32Resp[0] >> 16) & 0x0000ffff));
				xil_printf("\r\nManufacturing date          : %s %d",
						usz8Month, 0x7d0 + ((usz32Resp[0] >> 4) & 0x0000000f));
			}

			// sending CMD3
			do {
				u32Ret = fnSdSendCmd(CMD3, 0x00000000, 2, 0);
				if(u32Ret) {
					if(u8Verbose) {
						xil_printf("\r\nError: no response for CMD3 (R6).");
					}
					u16RetErr = 0xffff;
				}

				usz32Resp[0] = Xil_In32(reg.response_reg0);
				u16Rca = (usz32Resp[0] >> 16) & 0x0000ffff;

				if(u8Verbose) {
					xil_printf("\r\nRelative Card Address       : 0x%x", u16Rca);
				}
			}
			while(u16Rca == 0x0000);

			// display CSD contents
			if(u8Verbose) {
				fnSdSendCmd(CMD9, ((u16Rca << 16) & 0xffff0000), 1, 0);
				if(u32Ret) {
					xil_printf("\r\nError: no response for CMD2 (R2).");
					u16RetErr = 0xffff;
				}

				u8 read_bl_len, c_size_mult, file_format_grp, file_format, csd_ver;
				u32 c_size;

				usz32Resp[0] = Xil_In32(reg.response_reg0);
				usz32Resp[1] = Xil_In32(reg.response_reg1);
				usz32Resp[2] = Xil_In32(reg.response_reg2);
				usz32Resp[3] = Xil_In32(reg.response_reg3);

				csd_ver = ((usz32Resp[3] >> 22) & 0x00000003);
				xil_printf("\r\n\r\nCSD (ver. %d.0):", 1 + csd_ver);
				xil_printf("\r\nMax. data transfer rate     : %s",
						(((usz32Resp[2] >> 24) & 0x000000ff)==0x32)?"25 MHz":
						(((usz32Resp[2] >> 24) & 0x000000ff)==0x5a)?"50 MHz":
						(((usz32Resp[2] >> 24) & 0x000000ff)==0x0b)?"100 MHz":
						"200 MHz");
				xil_printf("\r\nCard command classes        : 0x%x",
						(usz32Resp[2] >> 12) & 0x00000fff);
				xil_printf("\r\nDSR implemented             : %s",
						(((usz32Resp[2] >> 4) & 0x00000001)==1)?"yes":"no");
				read_bl_len = (usz32Resp[1] >> 8) & 0x0000000f;
				xil_printf("\r\nMax. read data block length : %d",
						read_bl_len);
				c_size_mult = (usz32Resp[2] >> 7) & 0x00000007;
				xil_printf("\r\nDevice size multiplier      : %d",
						c_size_mult);
				c_size = (csd_ver == 0)?(((usz32Resp[2] << 10) & 0x00000c00) |
						((usz32Resp[1] >> 22) & 0x000003ff)):
						((usz32Resp[1] >> 8) & 0x007fffff);
				xil_printf("\r\nDevice size                 : %d", c_size);
				file_format_grp = (usz32Resp[0] >> 7) & 0x00000001;
				file_format = (usz32Resp[0] >> 2) & 0x00000003;
				xil_printf("\r\nFile Format                 : %s",
						((file_format_grp == 0) && (file_format == 0))?
						"Hard disk-like file system with partition table":
						((file_format_grp == 0) && (file_format == 1))?
						"DOS FAT with boot sector only (no partition table)":
						((file_format_grp == 0) && (file_format == 2))?
						"Universal file format":
						((file_format_grp == 0) && (file_format == 3))?
						"Others/Unknown":"Reserved");
			}
		}
	}

	if(u16RetErr == 0xffff) {
		return 0xffff;
	}
	else {
		return u16Rca;
	}
}

/******************************************************************************
 * Function to send a command.
 *
 * @param	u8Cmd is command to be sent.
 * @param	u32Arg is the argument of the command to be sent.
 * @param	u8RespType is the type of the response of the sent command.
 * @param	u8RecvData is the data received.
 *
 * @note	See SD Native standard 1.1
 *
 * @return	XST_SUCCESS - on success
 * 			XST_FAILURE - on failure
 *****************************************************************************/
XStatus fnSdSendCmd(u8 u8Cmd, u32 u32Arg, u8 u8RespType, u8 u8RecvData) {

	u32 u32RdData;

	// setting CMD index
	Xil_Out32(reg.command_reg, (u8Cmd << cmd_index) |
			(Xil_In32(reg.command_reg) & CMD_INDEX_MASK));
	// setting response type
	Xil_Out32(reg.command_reg, (u8RespType << resp_type) |
			(Xil_In32(reg.command_reg) & RESP_TYPE_MASK));
	// setting data transfer
	Xil_Out32(reg.command_reg, (u8RecvData << data_transfer) |
			(Xil_In32(reg.command_reg) & DATA_TRANSFER_MASK));
	// setting argument
	Xil_Out32(reg.argument_reg, u32Arg);

	// waiting for the command send to end
	do {
		u32RdData = Xil_In32(reg.status_reg);
	}
	while(!((u32RdData >> cmd_done) & 0x01));

	// waiting for the response to be received
	if(u8RespType) {
		do {
			u32RdData = Xil_In32(reg.status_reg);
			if((u32RdData >> resp_timeout) & 0x01) {
				if(u8Verbose) {
					xil_printf("\r\nResponse timeout.");
				}
				return XST_FAILURE;
			}
		}
		while(!((u32RdData >> resp_done) & 0x01));
	}

	// waiting for the data to be received
	if(u8RecvData) {
		do {
			u32RdData = Xil_In32(reg.status_reg);
		}
		while(!((u32RdData >> data_done) & 0x01));
	}

	return XST_SUCCESS;
}
