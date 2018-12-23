/******************************************************************************
 * @file video.h
 *
 * @authors Elod Gyorgy
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
 * 1.00  Elod Gyorgy  2015-Jan-15 First release
 *
 * </pre>
 *
 *****************************************************************************/

#ifndef VIDEO_H_
#define VIDEO_H_

#include "xstatus.h"
#include "xil_assert.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xtpg.h"
#include "xvtc.h"

/*
 * TPG defines
 */
#define H_ACTIVE_SIZE 1280
#define V_ACTIVE_SIZE 1024
#define INIT_PATTERN XTPG_TARAN_BARS

/*
 * Video path control gpio defines
 */
#define VID_MUX_PATTERN_INPUT 0
#define VID_MUX_DVI_INPUT 1

#define VID_MUX_DVI_SEL_BIT 0
#define VID_MUX_DP_SEL_BIT 1

#define VIDEO_PATH_CTRL_ID	XPAR_VIDEO_VIDEO_PATH_CTRL_DEVICE_ID
#define VIDEO_PATH_CTRL_INPUT_CHANNEL 1
#define VIDEO_PATH_CTRL_INTERRUPT XGPIO_IR_CH1_MASK  // Channel 1 Interrupt Mask
#define VIDEO_PATH_CTRL_OUTPUT_CHANNEL 2
#define VIDEO_PATH_CTRL_DVICLOCK_LOCKED_BIT_MASK 0x1
#define VIDEO_PATH_CTRL_VTD_LOCKED_BIT_MASK 0x2
#define VIDEO_PATH_CTRL_VIDOUT_LOCKED_BIT_MASK 0x4

/*
 * Convenience defines for video mux control
 */
#define fnSwitchDVItoPattern(psGpio) fnSetMux(psGpio, VID_MUX_DVI_SEL_BIT, VID_MUX_PATTERN_INPUT)
#define fnSwitchDVItoDVI(psGpio) fnSetMux(psGpio, VID_MUX_DVI_SEL_BIT, VID_MUX_DVI_INPUT)
#define fnSwitchDPtoPattern(psGpio) fnSetMux(psGpio, VID_MUX_DP_SEL_BIT, VID_MUX_PATTERN_INPUT)
#define fnSwitchDPtoDVI(psGpio) fnSetMux(psGpio, VID_MUX_DP_SEL_BIT, VID_MUX_DVI_INPUT)

XStatus fnTpgInit(XTpg *psTpg);
XStatus fnVtdInit(XVtc *psVtcInst);
XStatus fnInitVidMux(XGpio *psGpio);
void fnPS2MouseShow(int fVisible);
void fnSetMux(XGpio *psGpio, int dMuxId, int dInput);
void fnVideoPathCtrlIsr(void *CallbackRef);

#endif /* VIDEO_H_ */
