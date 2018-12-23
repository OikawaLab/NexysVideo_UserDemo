/******************************************************************************
 * @file oled.c
 * This is a lightweight implementation of the OLED driver taken from the
 * Linux PmodOLED driver.
 *
 * @authors Mihaita Nagy
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
 *
 * @note
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who          Date        Changes
 * ----- ------------ ----------- --------------------------------------------
 * 1.00  Mihaita Nagy 2015-Jan-15 First release
 *
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "oled.h"

/************************** Constant Definitions *****************************/

// OLED SPI and GPIO device IDs as defined in xparameters.h
#define OLED_SPI_DEVICE_ID			XPAR_SPI_1_DEVICE_ID
#define OLED_GPIO_DEVICE_ID			XPAR_OLED_AXI_GPIO_0_DEVICE_ID

// Mask of the OLED GPIO controller
#define OLED_VBAT_MASK				0x000001
#define OLED_VDD_MASK				0x000002
#define OLED_RES_MASK				0x000004
#define OLED_DC_MASK				0x000008

// Commands for the OLED controller
#define OLED_SET_PG_ADDR_CMD		0x22
#define OLED_DISPLAY_OFF_CMD		0xAE
#define OLED_DISPLAY_ON_CMD			0xAF
#define OLED_CONTRAST_CTRL_CMD		0x81
#define OLED_SET_PRECHARGE_PER_CMD	0xD9
#define OLED_SET_SEGMENT_REMAP_CMD	0xA0//0xA1//
#define OLED_SET_COM_DIR_CMD		0xC0//0xC8//
#define OLED_SET_COM_PINS_CMD		0xDA

// This is the offset from which the first ASCII character is mapped in the
// above table
#define ASCII_MAP_OFFSET			33

/**************************** Type Definitions *******************************/
// Type for the character array
typedef struct {
	u8 rgbPixel[8];
} sRow_t;

/************************** Variable Definitions *****************************/
// This variable holds the status of the current transfer
volatile u8 u8TransferInProg;

// Tracks any errors that occur during interrupt processing
u32 u32Error;

// This is the ASCII to pixel character map. In order to have an ASCII-compatible
// address mapping when addressed an offset of decimal 33 shall be used. This
// is mainly because only the text characters are mapped leaving out all the
// special characters.
const sRow_t AsciiMap[94] = {
	{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{{0x00, 0x00, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x00}},
	{{0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00}},
	{{0x64, 0x3c, 0x26, 0x64, 0x3c, 0x26, 0x24, 0x00}},
	{{0x26, 0x49, 0x49, 0x7f, 0x49, 0x49, 0x32, 0x00}},
	{{0x42, 0x25, 0x12, 0x08, 0x24, 0x52, 0x21, 0x00}},
	{{0x20, 0x50, 0x4e, 0x55, 0x22, 0x58, 0x28, 0x00}},
	{{0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00}},
	{{0x00, 0x00, 0x1c, 0x22, 0x41, 0x00, 0x00, 0x00}},
	{{0x00, 0x00, 0x00, 0x41, 0x22, 0x1c, 0x00, 0x00}},
	{{0x00, 0x15, 0x15, 0x0e, 0x0e, 0x15, 0x15, 0x00}},
	{{0x00, 0x08, 0x08, 0x3e, 0x08, 0x08, 0x00, 0x00}},
	{{0x00, 0x00, 0x00, 0x50, 0x30, 0x00, 0x00, 0x00}},
	{{0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00}},
	{{0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00}},
	{{0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00}},
	{{0x00, 0x3e, 0x41, 0x41, 0x41, 0x3e, 0x00, 0x00}},
	{{0x00, 0x00, 0x41, 0x7f, 0x40, 0x00, 0x00, 0x00}},
	{{0x00, 0x42, 0x61, 0x51, 0x49, 0x6e, 0x00, 0x00}},
	{{0x00, 0x22, 0x41, 0x49, 0x49, 0x36, 0x00, 0x00}},
	{{0x00, 0x18, 0x14, 0x12, 0x7f, 0x10, 0x00, 0x00}},
	{{0x00, 0x27, 0x49, 0x49, 0x49, 0x71, 0x00, 0x00}},
	{{0x00, 0x3c, 0x4a, 0x49, 0x48, 0x70, 0x00, 0x00}},
	{{0x00, 0x43, 0x21, 0x11, 0x0d, 0x03, 0x00, 0x00}},
	{{0x00, 0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00}},
	{{0x00, 0x06, 0x09, 0x49, 0x29, 0x1e, 0x00, 0x00}},
	{{0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00}},
	{{0x00, 0x00, 0x00, 0x52, 0x30, 0x00, 0x00, 0x00}},
	{{0x00, 0x00, 0x08, 0x14, 0x14, 0x22, 0x00, 0x00}},
	{{0x00, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00}},
	{{0x00, 0x00, 0x22, 0x14, 0x14, 0x08, 0x00, 0x00}},
	{{0x00, 0x02, 0x01, 0x59, 0x05, 0x02, 0x00, 0x00}},
	{{0x3e, 0x41, 0x5d, 0x55, 0x4d, 0x51, 0x2e, 0x00}},
	{{0x40, 0x7c, 0x4a, 0x09, 0x4a, 0x7c, 0x40, 0x00}},
	{{0x41, 0x7f, 0x49, 0x49, 0x49, 0x49, 0x36, 0x00}},
	{{0x1c, 0x22, 0x41, 0x41, 0x41, 0x41, 0x22, 0x00}},
	{{0x41, 0x7f, 0x41, 0x41, 0x41, 0x22, 0x1c, 0x00}},
	{{0x41, 0x7f, 0x49, 0x49, 0x5d, 0x41, 0x63, 0x00}},
	{{0x41, 0x7f, 0x49, 0x09, 0x1d, 0x01, 0x03, 0x00}},
	{{0x1c, 0x22, 0x41, 0x49, 0x49, 0x3a, 0x08, 0x00}},
	{{0x41, 0x7f, 0x08, 0x08, 0x08, 0x7f, 0x41, 0x00}},
	{{0x00, 0x41, 0x41, 0x7F, 0x41, 0x41, 0x00, 0x00}},
	{{0x30, 0x40, 0x41, 0x41, 0x3F, 0x01, 0x01, 0x00}},
	{{0x41, 0x7f, 0x08, 0x0c, 0x12, 0x61, 0x41, 0x00}},
	{{0x41, 0x7f, 0x41, 0x40, 0x40, 0x40, 0x60, 0x00}},
	{{0x41, 0x7f, 0x42, 0x0c, 0x42, 0x7f, 0x41, 0x00}},
	{{0x41, 0x7f, 0x42, 0x0c, 0x11, 0x7f, 0x01, 0x00}},
	{{0x1c, 0x22, 0x41, 0x41, 0x41, 0x22, 0x1c, 0x00}},
	{{0x41, 0x7f, 0x49, 0x09, 0x09, 0x09, 0x06, 0x00}},
	{{0x0c, 0x12, 0x21, 0x21, 0x61, 0x52, 0x4c, 0x00}},
	{{0x41, 0x7f, 0x09, 0x09, 0x19, 0x69, 0x46, 0x00}},
	{{0x66, 0x49, 0x49, 0x49, 0x49, 0x49, 0x33, 0x00}},
	{{0x03, 0x01, 0x41, 0x7f, 0x41, 0x01, 0x03, 0x00}},
	{{0x01, 0x3f, 0x41, 0x40, 0x41, 0x3f, 0x01, 0x00}},
	{{0x01, 0x0f, 0x31, 0x40, 0x31, 0x0f, 0x01, 0x00}},
	{{0x01, 0x1f, 0x61, 0x14, 0x61, 0x1f, 0x01, 0x00}},
	{{0x41, 0x41, 0x36, 0x08, 0x36, 0x41, 0x41, 0x00}},
	{{0x01, 0x03, 0x44, 0x78, 0x44, 0x03, 0x01, 0x00}},
	{{0x43, 0x61, 0x51, 0x49, 0x45, 0x43, 0x61, 0x00}},
	{{0x00, 0x00, 0x7f, 0x41, 0x41, 0x00, 0x00, 0x00}},
	{{0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00}},
	{{0x00, 0x00, 0x41, 0x41, 0x7f, 0x00, 0x00, 0x00}},
	{{0x00, 0x04, 0x02, 0x01, 0x01, 0x02, 0x04, 0x00}},
	{{0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00}},
	{{0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00}},
	{{0x00, 0x34, 0x4a, 0x4a, 0x4a, 0x3c, 0x40, 0x00}},
	{{0x00, 0x41, 0x3f, 0x48, 0x48, 0x48, 0x30, 0x00}},
	{{0x00, 0x3c, 0x42, 0x42, 0x42, 0x24, 0x00, 0x00}},
	{{0x00, 0x30, 0x48, 0x48, 0x49, 0x3f, 0x40, 0x00}},
	{{0x00, 0x3c, 0x4a, 0x4a, 0x4a, 0x2c, 0x00, 0x00}},
	{{0x00, 0x00, 0x48, 0x7e, 0x49, 0x09, 0x00, 0x00}},
	{{0x00, 0x26, 0x49, 0x49, 0x49, 0x3f, 0x01, 0x00}},
	{{0x41, 0x7f, 0x48, 0x04, 0x44, 0x78, 0x40, 0x00}},
	{{0x00, 0x00, 0x44, 0x7d, 0x40, 0x00, 0x00, 0x00}},
	{{0x00, 0x00, 0x40, 0x44, 0x3d, 0x00, 0x00, 0x00}},
	{{0x41, 0x7f, 0x10, 0x18, 0x24, 0x42, 0x42, 0x00}},
	{{0x00, 0x40, 0x41, 0x7f, 0x40, 0x40, 0x00, 0x00}},
	{{0x42, 0x7e, 0x02, 0x7c, 0x02, 0x7e, 0x40, 0x00}},
	{{0x42, 0x7e, 0x44, 0x02, 0x42, 0x7c, 0x40, 0x00}},
	{{0x00, 0x3c, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00}},
	{{0x00, 0x41, 0x7f, 0x49, 0x09, 0x09, 0x06, 0x00}},
	{{0x00, 0x06, 0x09, 0x09, 0x49, 0x7f, 0x41, 0x00}},
	{{0x00, 0x42, 0x7e, 0x44, 0x02, 0x02, 0x04, 0x00}},
	{{0x00, 0x64, 0x4a, 0x4a, 0x4a, 0x36, 0x00, 0x00}},
	{{0x00, 0x04, 0x3f, 0x44, 0x44, 0x20, 0x00, 0x00}},
	{{0x00, 0x02, 0x3e, 0x40, 0x40, 0x22, 0x7e, 0x40}},
	{{0x02, 0x0e, 0x32, 0x40, 0x32, 0x0e, 0x02, 0x00}},
	{{0x02, 0x1e, 0x62, 0x18, 0x62, 0x1e, 0x02, 0x00}},
	{{0x42, 0x62, 0x14, 0x08, 0x14, 0x62, 0x42, 0x00}},
	{{0x01, 0x43, 0x45, 0x38, 0x05, 0x03, 0x01, 0x00}},
	{{0x00, 0x46, 0x62, 0x52, 0x4a, 0x46, 0x62, 0x00}},
	{{0x00, 0x00, 0x08, 0x36, 0x41, 0x00, 0x00, 0x00}},
	{{0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00}},
	{{0x00, 0x00, 0x00, 0x41, 0x36, 0x08, 0x00, 0x00}}

};

// This is the Digilent logo defined as a 512-bit pixel array
const sPixmap_t sLogoPixmap = {{
	0xc0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0xe0, 0x70, 0x1c,
	0x06, 0x03, 0x09, 0x21, 0x83, 0x06, 0x1c, 0x70,
	0xe0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xc0,
	0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xe0, 0x30, 0x0c, 0x03, 0x81, 0x20, 0x08,
	0x02, 0xc1, 0x3c, 0x70, 0xc0, 0x02, 0x0c, 0x20,
	0x81, 0x03, 0x0c, 0x70, 0xc0, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x07, 0x07,
	0x07, 0x0f, 0x1e, 0xfe, 0xfc, 0xf8, 0x00, 0x00,
	0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,
	0xf8, 0xfc, 0xfe, 0x0f, 0x07, 0x07, 0xc7, 0xc7,
	0xcf, 0xce, 0xce, 0xc0, 0xc0, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff,
	0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xc7, 0xc7, 0xc7, 0xc7, 0xc7,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x3e, 0x7c,
	0xf0, 0xc0, 0x80, 0x00, 0xff, 0xff, 0xff, 0x00,
	0x00, 0x07, 0x07, 0x07, 0xff, 0xff, 0xff, 0x07,
	0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
	0xff, 0x00, 0x00, 0x00, 0x80, 0xe0, 0x38, 0x0c,
	0x03, 0x00, 0x20, 0x08, 0x02, 0xc0, 0xb8, 0x8c,
	0x83, 0x81, 0x80, 0x80, 0x80, 0x83, 0x8e, 0xb0,
	0xc0, 0x82, 0x04, 0x20, 0x81, 0x03, 0x0c, 0x30,
	0xc0, 0x00, 0x00, 0x3f, 0x3f, 0x3f, 0x38, 0x38,
	0x38, 0x3c, 0x1e, 0x1f, 0x0f, 0x07, 0x00, 0x00,
	0x00, 0x3f, 0x3f, 0x3f, 0x3f, 0x00, 0x00, 0x00,
	0x07, 0x0f, 0x1f, 0x3c, 0x38, 0x38, 0x31, 0x39,
	0x39, 0x1f, 0x1f, 0x07, 0x01, 0x00, 0x00, 0x00,
	0x3f, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x3f, 0x3f,
	0x3f, 0x3f, 0x38, 0x38, 0x38, 0x38, 0x00, 0x00,
	0x3f, 0x3f, 0x3f, 0x38, 0x38, 0x38, 0x38, 0x38,
	0x00, 0x00, 0x00, 0x3f, 0x3f, 0x3f, 0x00, 0x00,
	0x01, 0x07, 0x0f, 0x3e, 0x3f, 0x3f, 0x3f, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x3f, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
	0x00, 0x01, 0x09, 0x3f, 0x63, 0xc0, 0x90, 0x8c,
	0x82, 0x80, 0x80, 0x8a, 0x89, 0x88, 0x88, 0x88,
	0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
	0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
	0x88, 0x6b, 0x3f, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x7d, 0x55, 0x55,
	0x39, 0x01, 0x7d, 0x55, 0x55, 0x45, 0x01, 0x05,
	0x09, 0x71, 0x09, 0x05, 0x01, 0x39, 0x45, 0x45,
	0x45, 0x39, 0x01, 0x7d, 0x0d, 0x11, 0x61, 0x7d,
	0x01, 0x7d, 0x45, 0x45, 0x39, 0x01, 0x01, 0x01,
	0x05, 0x05, 0x7d, 0x05, 0x05, 0x01, 0x7d, 0x11,
	0x11, 0x7d, 0x01, 0x7d, 0x55, 0x55, 0x45, 0x01,
	0x39, 0x45, 0x45, 0x45, 0x39, 0x01, 0x7d, 0x15,
	0x35, 0x5d, 0x01, 0x05, 0x09, 0x71, 0x09, 0x05,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00
}};

/************************** Function Definitions *****************************/

/******************************************************************************
 * This function initializes the SPI and GPIO drivers needed to control the
 * OLED display.
 *
 * @param	psSpi is the pointer to the SPI driver structure.
 * @param	psGpio is the pointer to the GPIO driver structure.
 *
 * @return	XST_SUCCESS - Everything went well
 * 			XST_DEVICE_NOT_FOUND - Device not found
 * 			XST_FAILURE - Failure
 *****************************************************************************/
XStatus fnOledDriverInit(XSpi *psSpi, XGpio *psGpio) {

	XStatus Status;
	XSpi_Config *SpiConfigPtr;
	XGpio_Config *GpioConfigPtr;

	// Initialize the SPI driver so that it is  ready to use.
	SpiConfigPtr = XSpi_LookupConfig(OLED_SPI_DEVICE_ID);
	if(SpiConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XSpi_CfgInitialize(psSpi, SpiConfigPtr,
			SpiConfigPtr->BaseAddress);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Initialize the GPIO driver
	GpioConfigPtr = XGpio_LookupConfig(OLED_GPIO_DEVICE_ID);
	if(GpioConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XGpio_CfgInitialize(psGpio, GpioConfigPtr,
			GpioConfigPtr->BaseAddress);

	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************
 * This function sets the SPI interrupt handler and SPI options. This function
 * shall be used after the interrupts are registered.
 *
 * @param	psSpi is the pointer to the SPI driver structure.
 *
 * @return	XST_SUCCESS - Everything went well
 * 			XST_FAILURE - Failure
 *****************************************************************************/
XStatus fnOledDriverOptions(XSpi *psSpi) {

	XStatus Status;

	// Set the handler for the SPI that will be called from the interrupt
	// context
	XSpi_SetStatusHandler(psSpi, psSpi, (XSpi_StatusHandler)fnOledSpiIsr);

	// Set SPI device as master
	Status = XSpi_SetOptions(psSpi, XSP_MASTER_OPTION |
			XSP_MANUAL_SSELECT_OPTION);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Select the slave on the SPI bus (although the SS pin is not connected)
	Status = XSpi_SetSlaveSelect(psSpi, 0x01);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Start the SPI driver
	XSpi_Start(psSpi);

	return XST_SUCCESS;
}

/******************************************************************************
 * This function is the handler which performs processing for the SPI driver.
 *
 * @param	pvInst is a reference passed to the handler
 * @param	u32StatusEvent is the status of the SPI
 * @param	uwByteCnt is the number of bytes transferred
 *
 * @return	none
 *****************************************************************************/
void fnOledSpiIsr(void *pvInst, u32 u32StatusEvent, unsigned int uwByteCnt) {

	// Indicate the transfer on the SPI bus is no longer in progress
	// regardless of the status event
	u8TransferInProg = FALSE;

	if(u32StatusEvent != XST_SPI_TRANSFER_DONE) {
		u32Error++;
	}
}

/******************************************************************************
 * This function initializes the OLED display with the proper procedure.
 *
 * @param	psSpi is the pointer to the SPI driver structure.
 * @param	psGpio is the pointer to the GPIO driver structure.
 *
 * @return	XST_SUCCESS - Everything went well
 * 			XST_FAILURE - Failure
 *
 * @note	This is how the GPIO is mapped to the OLED controller:
 *				gpio(0) = vbat
 *				gpio(1) = vdd
 *				gpio(2) = res
 *				gpio(3) = dc
 *****************************************************************************/
XStatus fnOledDisplayInit(XSpi *psSpi, XGpio *psGpio) {

	XStatus Status;
	u8 rgbWrBuf[10];

	// Clear the data/cmd bit
	XGpio_DiscreteClear(psGpio, 1, OLED_DC_MASK);

	// Start by turning VDD on and wait for the power to come up
	XGpio_DiscreteClear(psGpio, 1, OLED_VDD_MASK);
	MB_Sleep(1);

	// Send to SPI the display off command
	rgbWrBuf[0] = OLED_DISPLAY_OFF_CMD;
	u8TransferInProg = TRUE;
	Status = XSpi_Transfer(psSpi, rgbWrBuf, NULL, 1);
	while(u8TransferInProg);

	// Bring reset low and then high
	XGpio_DiscreteSet(psGpio, 1, OLED_RES_MASK);
	MB_Sleep(1);
	XGpio_DiscreteClear(psGpio, 1, OLED_RES_MASK);
	MB_Sleep(1);
	XGpio_DiscreteSet(psGpio, 1, OLED_RES_MASK);

	// Send the set charge pump and set pre-charge period commands
	rgbWrBuf[0] = 0x8D;
	rgbWrBuf[1] = 0x14;
	rgbWrBuf[2] = OLED_SET_PRECHARGE_PER_CMD;
	rgbWrBuf[3] = 0xF1;
	u8TransferInProg = TRUE;
	Status = XSpi_Transfer(psSpi, rgbWrBuf, NULL, 4);
	while(u8TransferInProg);

	// Turn on VCC and wait 100 ms
	XGpio_DiscreteClear(psGpio, 1, OLED_VBAT_MASK);
	MB_Sleep(100);

	// Set display contrast
	rgbWrBuf[0] = OLED_CONTRAST_CTRL_CMD;
	rgbWrBuf[1] = 0x0F;

	// Invert the display
	rgbWrBuf[2] = OLED_SET_SEGMENT_REMAP_CMD;
	rgbWrBuf[3] = OLED_SET_COM_DIR_CMD;

	// Select sequential COM configuration
	rgbWrBuf[4] = OLED_SET_COM_PINS_CMD;
	rgbWrBuf[5] = 0x00;
	rgbWrBuf[6] = 0xC0;
	rgbWrBuf[7] = 0x20;
	rgbWrBuf[8] = 0x00;

	// Turn display on
	rgbWrBuf[9]/*[6]*/ = OLED_DISPLAY_ON_CMD;

	u8TransferInProg = TRUE;
	Status = XSpi_Transfer(psSpi, rgbWrBuf, NULL, 10);
	while(u8TransferInProg);

	//Display Digilent logo
	fnOledPixelToDisplay(psSpi, psGpio, &sLogoPixmap);

	return Status;
}

/******************************************************************************
 * This function displays the pixel buffer, pointed to with psScreenBuf, onto
 * the OLED display.
 *
 * @param	psSpi is the pointer to the SPI driver structure.
 * @param	psGpio is the pointer to the GPIO driver structure.
 * @param	pchScreenBuf is the pointer to the pixel buffer to display on the
 * 			OLED screen.
 *
 * @return	XST_SUCCESS - Everything went well
 * 			XST_FAILURE - Failure
 *****************************************************************************/
XStatus fnOledPixelToDisplay(XSpi *psSpi, XGpio *psGpio,
		const sPixmap_t *psScreenBuf) {

	XStatus Status;
	u32 iPg;
	u8 rgbWrBuf[10];
	u8 u8LowerStartColumn = 0x00;
	u8 u8UpperStartColumn = 0x10;

	// Going through each character line (4)
	for(iPg = 0; iPg < OLED_MAX_PG_CNT; iPg++) {

		rgbWrBuf[0] = OLED_SET_PG_ADDR_CMD;
		rgbWrBuf[1] = iPg;
		rgbWrBuf[2] = u8LowerStartColumn;
		rgbWrBuf[3] = u8UpperStartColumn;

		XGpio_DiscreteClear(psGpio, 1, OLED_DC_MASK);

		u8TransferInProg = TRUE;
		Status = XSpi_Transfer(psSpi, rgbWrBuf, NULL, 4);
		if(Status != XST_SUCCESS) {
			return Status;
		}
		while(u8TransferInProg);

		XGpio_DiscreteSet(psGpio, 1, OLED_DC_MASK);

		// Writing the line to the OLED
		u8TransferInProg = TRUE;
		Status = XSpi_Transfer(psSpi, (u8 *)(psScreenBuf->rgbPixmap +
				(iPg * OLED_CONTROLLER_PG_SZ)), NULL, OLED_CONTROLLER_PG_SZ);
		if(Status != XST_SUCCESS) {
			return Status;
		}
		while(u8TransferInProg);
	}

	return XST_SUCCESS;
}

/******************************************************************************
 * This function displays a certain string (pointed with pchScreenBuf) on the
 * OLED display. It can display a maximum of 16 characters per line and a total
 * of 4 lines.
 *
 * @param	psSpi is the pointer to the SPI driver structure.
 * @param	psGpio is the pointer to the GPIO driver structure.
 * @param	pchScreenBuf is the pointer to the buffer to display on the OLED
 * 			screen.
 *
 * @return	XST_SUCCESS - Everything went well
 * 			XST_FAILURE - Failure
 *****************************************************************************/
XStatus fnOledStringToDisplay(XSpi *psSpi, XGpio *psGpio,
		const char *pchScreenBuf) {

	u32 iConv;
	u16 u16CntCharBuf;
	sPixmap_t sNewBuf;

	// Check the buffer size to see if it's out of range
	if(strlen(pchScreenBuf) > OLED_DISPLAY_BUF_SZ/sizeof(sRow_t)) {
		u16CntCharBuf = OLED_DISPLAY_BUF_SZ/sizeof(sRow_t);
	}
	else {
		u16CntCharBuf = strlen(pchScreenBuf);
	}

	// Clear screen buffer
	memset(&sNewBuf, 0, sizeof(sNewBuf));

	// This for iterates through each character
	for(iConv = 0; iConv < u16CntCharBuf; iConv++) {

		char chTempChar;

		// Check for the current char to see if it's out of range
		if((*pchScreenBuf < ' ') || (*pchScreenBuf >= ASCII_MAP_OFFSET +
				sizeof(AsciiMap)/sizeof(sRow_t))) {
			chTempChar = ' ';
		}
		else {
			chTempChar = *pchScreenBuf;
		}

		// Convert the character to its pixel correspondence
		memcpy((&sNewBuf.rgbPixmap[sizeof(sRow_t) * iConv]),
				(void *)&AsciiMap[chTempChar - (ASCII_MAP_OFFSET - 1)],
				sizeof(sRow_t));

		// Increment to next character
		pchScreenBuf++;
	}

	// Send screen buffer to OLED for display
	return fnOledPixelToDisplay(psSpi, psGpio, &sNewBuf);
}

