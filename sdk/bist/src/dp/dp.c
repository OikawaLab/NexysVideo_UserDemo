/******************************************************************************
 * @file dp.c
 * DisplayPort Policy Maker
 *
 * @author Mihaita Nagy
 *
 * @date 2014-Oct-30
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
 * Contains two functions: one that initializes the Video Timing Controller
 * and another one that implements Xilinx's stripped-out Policy Maker. The
 * latter uses the freely-taken Policy Maker functions from the XAPP1178.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who          Date     Changes
 * ----- ------------ ----------- -----------------------------------------------
 * 1.00  Mihaita Nagy 2014-Nov-20 First release
 *
 * </pre>
 *
 *****************************************************************************/
/***************************** Include Files *********************************/
#include "dp.h"
#include "../video/video.h"

/************************** Variable Definitions *****************************/
extern unsigned char u8Verbose;

/************************** Function Definitions *****************************/

/******************************************************************************
 * This function calls the Policy Maker and negotiates DisplayPort parameters.
 *
 * @param	psVtcInst is the pointer to the Video Timing Controller driver
 * 			instance.
 *
 * @return	00 - PASS
 * 			FF - FAIL
 *****************************************************************************/
XStatus fnDpTest(XVtc *psVtcInst) {

	XILDPMainStreamAttributes xilMSA;
	XVtc_Timing Timing;
	u16 oldHActive = 0, oldVActive = 0;
	static u32 vm = 9;

	if(u8Verbose) {
		xil_printf("\r\nStarting the DisplayPort demo...");
	}

	// Initializing the DisplayPort core
	xildpInitDisplayport();

	// Set up defaults for the link policy maker
	dplpmInitLinkPolicyMaker(XILINX_DISPLAYPORT_LINK_RATE_2_7GBPS, 2);

	// Waiting for HPD to occur and Link Training to finish
	while(dplpmCheckHPDStatus() != HPD_STATE_CONNECTED);

	/*dplpmMainLinkEnable(0);

	// Toggling reset
	dptx_reg_write(XILINX_DISPLAYPORT_TX_SOFT_VIDEO_RESET, 0xF);
	dptx_reg_write(XILINX_DISPLAYPORT_TX_SOFT_VIDEO_RESET, 0x0);

	// Clearing MSA values
	xildpClearMSAValues();

	xilMSA.user_pixel_width 	= 1;
	xilMSA.h_clk_total 			= dmt_modes[vm].hsw + dmt_modes[vm].hfp + dmt_modes[vm].hres + dmt_modes[vm].hbp;
	xilMSA.v_clk_total 			= dmt_modes[vm].vsw + dmt_modes[vm].vfp + dmt_modes[vm].vres + dmt_modes[vm].vbp;
	xilMSA.v_sync_polarity 		= dmt_modes[vm].vpol;
	xilMSA.h_sync_polarity 		= dmt_modes[vm].hpol;
	xilMSA.h_sync_width 		= dmt_modes[vm].hsw;
	xilMSA.v_sync_width 		= dmt_modes[vm].vsw;
	xilMSA.h_resolution 		= dmt_modes[vm].hres;
	xilMSA.v_resolution 		= dmt_modes[vm].vres;
	xilMSA.h_start 				= dmt_modes[vm].hsw + dmt_modes[vm].hbp;
	xilMSA.v_start 				= dmt_modes[vm].vsw + dmt_modes[vm].vbp;
	xilMSA.misc0 				= 0x21;
	xilMSA.misc1				= 0x00;
	xilMSA.m_vid 				= dmt_modes[vm].vclk;
	xilMSA.n_vid 				= 270000; // lnk_clk * 2 [kHz]
	xilMSA.data_per_lane 		= (dmt_modes[vm].hres*24/16)-2;
	xilMSA.transfer_unit_size 	= xilSetTU(dmt_modes[vm].vclk, 8*3, XILINX_DISPLAYPORT_LINK_RATE_2_7GBPS, 2, 0);

	// Writing MSA values
	xildpSetMSAValues(&xilMSA);

	// Enabling main video
	dplpmMainLinkEnable(1);

	if(u8Verbose) {
		xil_printf("\r\n...");
	}*/

	// Auto-change resolution
	while(1) {

		// Reading input video parameters
		XVtc_GetDetectorTiming(psVtcInst, &Timing);

		if((oldHActive != Timing.HActiveVideo) & (oldVActive != Timing.VActiveVideo) &
		  ((XVtc_ReadReg(psVtcInst->Config.BaseAddress, XVTC_ISR_OFFSET) & XVTC_IXR_LO_MASK) ==
		  XVTC_IXR_LO_MASK)){

			// Disabling main stream to force sending of IDLE patterns
			dplpmMainLinkEnable(0);

			// Toggling reset
			dptx_reg_write(XILINX_DISPLAYPORT_TX_SOFT_VIDEO_RESET, 0xF);
			dptx_reg_write(XILINX_DISPLAYPORT_TX_SOFT_VIDEO_RESET, 0x0);

			// Clearing MSA values
			xildpClearMSAValues();

			// Reading and setting the new video timing values
			xilMSA.user_pixel_width 	= 1;
			xilMSA.h_clk_total 			= Timing.HSyncWidth + Timing.HFrontPorch + Timing.HActiveVideo + Timing.HBackPorch;
			xilMSA.v_clk_total 			= Timing.V0SyncWidth + Timing.V0FrontPorch + Timing.VActiveVideo + Timing.V0BackPorch;
			xilMSA.v_sync_polarity 		= Timing.VSyncPolarity;
			xilMSA.h_sync_polarity 		= Timing.HSyncPolarity;
			xilMSA.h_sync_width 		= Timing.HSyncWidth;
			xilMSA.v_sync_width 		= Timing.V0SyncWidth;
			xilMSA.h_resolution 		= Timing.HActiveVideo;
			xilMSA.v_resolution 		= Timing.VActiveVideo;
			xilMSA.h_start 				= Timing.HSyncWidth + Timing.HBackPorch;
			xilMSA.v_start 				= Timing.V0SyncWidth + Timing.V0BackPorch;
			xilMSA.misc0 				= 0x21;
			xilMSA.misc1				= 0x00;
			xilMSA.m_vid 				= fnResolutionToPclk(Timing.HActiveVideo, Timing.VActiveVideo);
			xilMSA.n_vid 				= 270000; // lnk_clk * 2 [kHz]
			xilMSA.data_per_lane 		= (Timing.HActiveVideo*24/16)-2; // (xilVPR.horizontal_resolutions *  bpc * 3 /16) - mlconf->lane_count;
			xilMSA.transfer_unit_size 	= xilSetTU(xilMSA.m_vid, 8*3, XILINX_DISPLAYPORT_LINK_RATE_2_7GBPS, 2, 0);

			// Writing MSA values
			xildpSetMSAValues(&xilMSA);

			// Enabling main video
			dplpmMainLinkEnable(1);

			xil_printf("\r\nVideo resolution: %dx%d/%d kHz", Timing.HActiveVideo,
					Timing.VActiveVideo, fnResolutionToPclk(Timing.HActiveVideo,
					Timing.VActiveVideo));
			break;
		}
		oldHActive = Timing.HActiveVideo;
	}

	// Disabling VTC Detector
	XVtc_DisableDetector(psVtcInst);

	return XST_SUCCESS;
}

/******************************************************************************
 * Simple look-up table that returns the pixel frequency in kHz for the given
 * horizontal (HActive) and vertical (VActive) video resolutions.
 *
 * @param	HActive is the horizontal active video pixels.
 * @param	VActive is the vertical active video pixels.
 *
 * @return	The corresponding 32-bit unsigned pixel frequency.
 *****************************************************************************/
u32 fnResolutionToPclk(u32 u32HActive, u32 u32VActive) {

	u32 u32Clk;

	switch(u32HActive) {
		case 1920:
			if (u32VActive == 1080) {
				u32Clk = 148500;
			}
			break;
		case 1680:
			if (u32VActive == 1050) {
				u32Clk = 147140;
			}
			break;
		case 1440:
			if (u32VActive == 900) {
				u32Clk = 106470;
			}
			break;
		case 1400:
			if (u32VActive == 1050) {
				u32Clk = 122610;
			}
			break;
		case 1366:
			if (u32VActive == 768) {
				u32Clk =  85860;
			}
			break;
		case 1360:
			if (u32VActive == 768) {
				u32Clk =  85500;
			}
			break;
		case 1280:
			if (u32VActive == 1024) {
				//u32Clk = 108000;
				u32Clk = 107954;
			}
			else if (u32VActive == 960) {
				u32Clk = 102100;
			}
			else if (u32VActive == 800) {
				u32Clk =  83460;
			}
			else if (u32VActive == 768) {
				u32Clk =  68250;
			}
			else if (u32VActive == 720) {
				u32Clk =  74250;
			}
			break;
		case 1152:
			if (u32VActive == 864) {
				u32Clk =  81620;
			}
			break;
		case 1024:
			if (u32VActive == 768) {
				u32Clk =  65000;
			}
			break;
		case 800:
			if (u32VActive == 600) {
				u32Clk =  40000;
			}
			break;
		default:
			u32Clk = 108000;
			break;
	}

	return u32Clk;
}

