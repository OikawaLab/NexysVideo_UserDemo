/******************************************************************************
 * @file bootloader.c
 * SREC Bootloader implementation on a Spansion serial flash medium.
 *
 * @authors Elod Gyorgy
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
 * This bootloader will call the QSPI init functions and then read the SREC
 * image line-by-line from address FLASH_IMAGE_BASEADDR. It decodes the SREC
 * image and copies the application to DDR memory. It then hands over control
 * to the booted application.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who          Date        Changes
 * ----- ------------ ----------- --------------------------------------------
 * 1.01  Elod Gyorgy  2015-Jan-21 Warm-reset fix
 * 1.00  Elod Gyorgy  2015-Dec-23 First release
 *
 *
 * </pre>
 *
 *****************************************************************************/

/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 *      Simple SREC Bootloader
 *      This simple bootloader is provided with Xilinx EDK for you to easily re-use in your
 *      own software project. It is capable of booting an SREC format image file 
 *      (Mototorola S-record format), given the location of the image in memory.
 *      In particular, this bootloader is designed for images stored in non-volatile flash
 *      memory that is addressable from the processor. 
 *
 *      Please modify the define "FLASH_IMAGE_BASEADDR" in the blconfig.h header file 
 *      to point to the memory location from which the bootloader has to pick up the 
 *      flash image from.
 *
 *      You can include these sources in your software application project in XPS and 
 *      build the project for the processor for which you want the bootload to happen.
 *      You can also subsequently modify these sources to adapt the bootloader for any
 *      specific scenario that you might require it for.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blconfig.h"
#include "portab.h"
#include "errors.h"
#include "srec.h"
#include "qspi.h"
#include "platform.h"

/* Defines */
#define CR       13

/* Comment the following line, if you want a smaller and faster bootloader which will be silent */
//#define VERBOSE
#define INFO

/* Declarations */
static void display_progress (uint32 lines);
static uint8 load_exec ();
static uint8 flash_get_srec_line (uint8 *buf);
extern void init_stdout();

extern int srec_line;

#ifdef __cplusplus
extern "C" {
#endif

extern void outbyte(char c); 

#ifdef __cplusplus
}
#endif

/* Data structures */
static srec_info_t srinfo;
static uint8 sr_buf[SREC_MAX_BYTES];
static uint8 sr_data_buf[SREC_DATA_MAX_BYTES];

static uint8 *flbuf;

static XSpi sQSpi;
extern u8 rgbReadBuffer[PAGE_SIZE + READ_WRITE_EXTRA_BYTES];
static unsigned int ibBuffer;
static u32 dwFlashAddr;

#ifdef VERBOSE
static int8 *errors[] = {
    "",
    "Error while copying executable image into RAM",
    "Error while reading an SREC line from flash",
    "SREC line is corrupted",
    "SREC has invalid checksum."
};
#endif

/* We don't use interrupts/exceptions. 
   Dummy definitions to reduce code size on MicroBlaze */
#ifdef __MICROBLAZE__
void _interrupt_handler () {}
void _exception_handler () {}
void _hw_exception_handler () {}
#endif


int main()
{
    uint8 ret;

	// We cannot rely on initialized globals here. Explicitely initialized globals go
	// into the .data section, and changes to these variables are carried over to the
    // next run when the processor is warm-reset.
    // Implicitly initialized (to zero) globals should be properly set by the C runtime
    // before main() is run, so we shouldn't worry about those
    // Do explicit inits here.
	dwFlashAddr = FLASH_IMAGE_BASEADDR;

    // Disable caches so that the booted application starts correctly
    // We don't need caches here since we are running from BRAM
    init_platform();

#ifdef INFO
    print("\033[H\033[J"); //Clear the terminal
    print("\r\nBootloader started.");
#endif

    if (XST_SUCCESS != init_qspi(&sQSpi))
    {
#ifdef VERBOSE
    	print("\r\nQuad SPI init failed.");
#endif
    	ret = QSPI_ERROR;
    }
    else
    {

#ifdef VERBOSE    
		print ("\r\nSREC Bootloader\r\n");
		print ("Loading SREC image from flash @ address: ");
		putnum (dwFlashAddr);
		print ("\r\n");
#endif

		flbuf = (uint8*)dwFlashAddr;
		ret = load_exec ();
    }
    /* If we reach here, we are in error */
#ifdef INFO
    print("\r\nBootloader error.");
#endif
#ifdef VERBOSE
    if (ret > LD_SREC_LINE_ERROR) {
        print ("ERROR in SREC line: ");
        putnum (srec_line);
        print (errors[ret]);    
    } else {
        print ("ERROR: ");
        print (errors[ret]);
    }
#endif

    return ret;
}

#ifdef VERBOSE
static void display_progress (uint32 count)
{
    /* Send carriage return */
    outbyte (CR);  
    print  ("Bootloader: Processed (0x)");
    putnum (count);
    print (" S-records");
}
#endif

static uint8 load_exec ()
{
    uint8 ret;
    void (*laddr)();
    int8 done = 0;
    
    srinfo.sr_data = sr_data_buf;
    
    while (!done) {
        if ((ret = flash_get_srec_line (sr_buf)) != 0) 
            return ret;

        if ((ret = decode_srec_line (sr_buf, &srinfo)) != 0)
            return ret;
        
#ifdef VERBOSE
        display_progress (srec_line);
#endif
        switch (srinfo.type) {
            case SREC_TYPE_0:
                break;
            case SREC_TYPE_1:
            case SREC_TYPE_2:
            case SREC_TYPE_3:
                memcpy ((void*)srinfo.addr, (void*)srinfo.sr_data, srinfo.dlen);
                break;
            case SREC_TYPE_5:
                break;
            case SREC_TYPE_7:
            case SREC_TYPE_8:
            case SREC_TYPE_9:
                laddr = (void (*)())srinfo.addr;
                done = 1;
                ret = 0;
                break;
        }
    }

#ifdef VERBOSE
    print ("\r\nExecuting program starting at address: ");
    putnum ((uint32)laddr);
    print ("\r\n");
#endif

#ifdef INFO
    print("\r\nBooting...");
#endif

    cleanup_platform();

    (*laddr)();                 
  
    /* We will be dead at this point */
    return 0;
}


static uint8 flash_get_srec_line (uint8 *buf)
{
    uint8 c;
    int count = 0;

    while (1) {
    	if (ibBuffer == 0) {
    		//Buffer is empty, read from Flash
    		if (XST_SUCCESS != SpiFlashRead(&sQSpi, dwFlashAddr, PAGE_SIZE))
    		{
    			return QSPI_ERROR;
    		}
    		ibBuffer = PAGE_SIZE;
    		dwFlashAddr += PAGE_SIZE;

    		flbuf = &rgbReadBuffer[READ_WRITE_EXTRA_BYTES];
    	}

        c  = *flbuf++; ibBuffer--;
        if (c == 0xD) {   
            /* Eat up the 0xA too */
        	if (ibBuffer == 0)
        	{
        		// If the buffer just emptied, skip the next byte for the next read
        		dwFlashAddr++;
        	}
        	else
        	{
        		// Skip the next byte
        		c = *flbuf++; ibBuffer--;
        	}
            return 0;
        }
        
        *buf++ = c;
        count++;
        if (count > SREC_MAX_BYTES) 
            return LD_SREC_LINE_ERROR;
    }
}

#ifdef __PPC__

#include <unistd.h>

/* Save some code and data space on PowerPC 
   by defining a minimal exit */
void exit (int ret)
{
    _exit (ret);
}
#endif
