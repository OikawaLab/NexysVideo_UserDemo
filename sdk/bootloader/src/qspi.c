/******************************************************************************
 * @file qspi.c
 * Bootloader implementation on a Spansion serial flash medium.
 *
 * @authors Elod Gyorgy
 *
 * @date 2015-Dec-23
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
 * The SREC bootloader will call these functions to read data from the flash.
 * These functions use the Xilinx Quad SPI IP driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who          Date        Changes
 * ----- ------------ ----------- --------------------------------------------
 * 1.00  Elod Gyorgy  2015-Dec-23 First release
 *
 *
 * </pre>
 *
 *****************************************************************************/

#include "qspi.h"
#include "xparameters.h"

#define QSPI_DEVICE_ID	XPAR_AXI_QUAD_SPI_0_DEVICE_ID
#define QSPI_SS_MASK	0x1	//Slave select 0 is tied to the SS of the flash

#define PAGE_SIZE 	256 //Also maximum data byte count
/*
 * Definitions of the commands shown in this example.
 */
#define COMMAND_RDSR1			0x05 /* Status read command */
#define COMMAND_RDCR			0x35 /* Status read command */
#define COMMAND_4QOR			0x6C /* Read Quad Out (4-byte Address) */

/**
 * This definitions number of bytes in each of the command
 * transactions. This count includes Command byte, address bytes and any
 * don't care bytes needed.
 */
#define RDSR1_BYTES				2 /* Status read bytes count */
#define RDCR_BYTES				2 /* Status read bytes count */

/*
 * Flash not busy mask in the status register of the flash device.
 */
#define FLASH_SR_IS_READY_MASK		0x01 /* Ready mask */

/*
 * Byte Positions.
 */
#define BYTE1				0 /* Byte 1 position */
#define BYTE2				1 /* Byte 2 position */
#define BYTE3				2 /* Byte 3 position */
#define BYTE4				3 /* Byte 4 position */
#define BYTE5				4 /* Byte 5 position */
#define BYTE6				5 /* Byte 6 position */
#define BYTE7				6 /* Byte 7 position */
#define BYTE8				7 /* Byte 8 position */

/*
 * Buffers used during read and write transactions.
 */
u8 rgbReadBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES];
u8 rgbWriteBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES];

XStatus init_qspi(XSpi *psQSpi)
{
	int Status;
	XSpi_Config *pConfigPtr;	/* Pointer to Configuration data */

    /* Init Quad SPI too
     * Since the Quad SPI Flash is not memory-mapped, memory access instructions
     * need to be replaced by function calls to the AXI Quad SPI core to fetch data */


	/*
	 * Initialize the SPI driver so that it is  ready to use.
	 */
	pConfigPtr = XSpi_LookupConfig(QSPI_DEVICE_ID);
	if (pConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XSpi_CfgInitialize(psQSpi, pConfigPtr,
			pConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly
	 */
	Status = XSpi_SelfTest(psQSpi);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the SPI device as a master and in manual slave select mode such
	 * that the slave select signal does not toggle for every byte of a
	 * transfer, this must be done before the slave select is set.
	 */
	Status = XSpi_SetOptions(psQSpi, XSP_MASTER_OPTION |
				 XSP_MANUAL_SSELECT_OPTION);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the SPI driver.
	 */
	XSpi_Start(psQSpi);

	/*
	 * Disable Global interrupt to use polled mode operation
	 */
	XSpi_IntrGlobalDisable(psQSpi);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads the data from the Winbond Serial Flash Memory
*
* @param	SpiPtr is a pointer to the instance of the Spi device.
* @param	Addr is the starting address in the Flash Memory from which the
*		data is to be read.
* @param	ByteCount is the number of bytes to be read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None
*
******************************************************************************/
XStatus SpiFlashRead(XSpi *psQSpi, u32 Addr, u32 ByteCount)
{
	int Status;

	if (ByteCount > PAGE_SIZE)
		return XST_FAILURE;

	/*
	 * Select the quad flash device on the SPI bus, so that it can be
	 * read and written using the SPI bus.
	 */
	Status = XSpi_SetSlaveSelect(psQSpi, QSPI_SS_MASK);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait while the Flash is busy.
	 */
	Status = SpiFlashWaitForFlashReady(psQSpi);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = SpiFlashGetControl(psQSpi);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Prepare the rgbWriteBuffer.
	 */
	rgbWriteBuffer[BYTE1] = COMMAND_4QOR;
	rgbWriteBuffer[BYTE2] = (u8) (Addr >> 24);
	rgbWriteBuffer[BYTE3] = (u8) (Addr >> 16);
	rgbWriteBuffer[BYTE4] = (u8) (Addr >> 8);
	rgbWriteBuffer[BYTE5] = (u8) (Addr);

	/*
	 * Initiate the Transfer.
	 */
	do
	{
		Status = XSpi_Transfer(psQSpi, &rgbWriteBuffer[0], &rgbReadBuffer[0],
				(ByteCount + READ_WRITE_EXTRA_BYTES));
	}
	while (Status == XST_DEVICE_BUSY);

	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Select the quad flash device on the SPI bus, so that it can be
	 * read and written using the SPI bus.
	 */
	Status = XSpi_SetSlaveSelect(psQSpi, 0);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function waits until the flash is ready to accept the next
* command.
*
* @param	None
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		This function reads the status register of the Buffer and waits
*.		till the WIP bit of the status register becomes 0.
*
******************************************************************************/
XStatus SpiFlashWaitForFlashReady(XSpi *psQSpi)
{
	int Status;
	u8 StatusReg;

	while(1) {

		/*
		 * Get the Status Register.
		 */
		Status = SpiFlashGetStatus(psQSpi);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Check if the flash is ready to accept the next command.
		 * If so break.
		 */
		StatusReg = rgbReadBuffer[1];
		if((StatusReg & FLASH_SR_IS_READY_MASK) == 0) {
			break;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads the Status register SR1
*
* @param	SpiPtr is a pointer to the instance of the Spi device.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The status register content is stored at the second byte pointed
*		by the rgbReadBuffer.
*
******************************************************************************/
XStatus SpiFlashGetStatus(XSpi *psQSpi)
{
	int Status;

	/*
	 * Prepare the Write Buffer.
	 */
	rgbReadBuffer[BYTE1] = COMMAND_RDSR1;

	/*
	 * Initiate the Transfer.
	 */
	Status = XSpi_Transfer(psQSpi, &rgbReadBuffer[0], &rgbReadBuffer[0],
			RDSR1_BYTES);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads the Control register CR1
*
* @param	SpiPtr is a pointer to the instance of the Spi device.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The status register content is stored at the second byte pointed
*		by the rgbReadBuffer.
*
******************************************************************************/
XStatus SpiFlashGetControl(XSpi *psQSpi)
{
	int Status;

	/*
	 * Prepare the Write Buffer.
	 */
	rgbReadBuffer[BYTE1] = COMMAND_RDCR;

	/*
	 * Initiate the Transfer.
	 */
	Status = XSpi_Transfer(psQSpi, &rgbReadBuffer[0], &rgbReadBuffer[0],
			RDCR_BYTES);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
