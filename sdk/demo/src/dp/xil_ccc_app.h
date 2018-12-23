/* ***************************************************************************
 * ** Copyright (c) 2012-2013 Xilinx, Inc.  All rights reserved.            **
 * **   ____  ____                                                          **
 * **  /   /\/   /                                                          **
 * ** /___/  \  /   Vendor: Xilinx                                          **
 * ** \   \   \/                                                            **
 * **  \   \                                                                **
 * **  /   /                                                                **
 * ** /___/   /\                                                            **
 * ** \   \  /  \                                                           **
 * **  \___\/\___\                                                          **
 * **                                                                       **
 * ** XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"         **
 * ** AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND       **
 * ** SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,        **
 * ** OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,        **
 * ** APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION           **
 * ** THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,     **
 * ** AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE      **
 * ** FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY              **
 * ** WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE               **
 * ** IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR        **
 * ** REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF       **
 * ** INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS       **
 * ** FOR A PARTICULAR PURPOSE.                                             **
 * **                                                                       **
 * **************************************************************************/


#ifndef __XILINX_COMMAND_CONTROL_COMMUNICATION_APP__
#define __XILINX_COMMAND_CONTROL_COMMUNICATION_APP__

#include "xil_types.h"
#include "displayport_defs.h"

#define   RUN_DMT_MODES            1


typedef struct
{
    char cmd_key;
    UINT32 enable_vcap;
    UINT32 enable_loopthrough_video;
    UINT32 enable_single_frame_capture;
} XILCCCAppControl;


typedef struct 
{
  UINT32 vsync_polarity;
  UINT32 hsync_polarity;
  UINT32 de_polarity;
  UINT32 vsync_pulse_width;
  UINT32 vertical_back_porch;
  UINT32 vertical_front_porch;
  UINT32 vertical_resolutions;
  UINT32 hsync_pulse_width;
  UINT32 horizontal_back_porch;
  UINT32 horizontal_front_porch;
  UINT32 horizontal_resolutions;
  UINT32 framelock0; //bit21:0-framelock delay; bit31-framelock enable
  UINT32 framelock1; //bit10:0-framelock line fraction; bit16-framelock align hsync
  UINT32 hdcolorbar_mode; //address 0x40

  UINT32 tc_hsblnk;
  UINT32 tc_hssync;
  UINT32 tc_hesync;
  UINT32 tc_heblnk;
  UINT32 tc_vsblnk;
  UINT32 tc_vssync;
  UINT32 tc_vesync;
  UINT32 tc_veblnk;

  UINT32 vid_clk_sel; //address 0x100
  UINT32 vid_clk_M;  //M value to be used for vid_clk synthesis
  UINT32 vid_clk_D;  //D value to be used for vid_clk synthesis

}XilVideoPatternRegisters;


typedef struct
{
  char*  name;
  UINT8  id;
  UINT16 hres;
  UINT16 vres;
  UINT32 vclk;
  UINT8  scan;
  UINT8  hpol;
  UINT8  vpol;
  UINT32 hfp;
  UINT32 hsw;
  UINT32 hbp;
  UINT32 vfp;
  UINT32 vsw;
  UINT32 vbp;
} DMT_MODES;


UINT32 xilcccAppInit(void);
void   init_video_modes(void);
UINT32 xilcccAppRunLoop(void);
UINT32 xilcccCommandProcessor(char command_key);
void   xilcccDisplayHelp(void);
void   xilcccModeHelp(void);
UINT32 xilcccChangeVideoMode(UINT8 inc, UINT8 set_mode, UINT8 mode, UINT8 bpc);
void   xilSetPatternGeneratorRegisters(XilVideoPatternRegisters *);

void   xilComputeVidMVid ( UINT32 vid_freq, UINT32 ref_freq, XilVideoPatternRegisters *ptrVideoPat);
UINT32 xilSetTU (UINT32 vid_clk, UINT32 bpp, UINT32 link_rate, UINT32 lanes, UINT8 mst_enable);
void read_allocation_table(void);
void   wait_tx_vsyncs (UINT32 value );


#endif
