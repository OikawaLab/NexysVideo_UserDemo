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

#ifndef __XILINX_DISPLAYPORT_LINK_POLICY_MAKER__
#define __XILINX_DISPLAYPORT_LINK_POLICY_MAKER__

#include "edid_drv.h"
#include "mode_table.h"
#include "xil_displayport.h"

#define XILINX_DISPLAYPORT_LPM_STATUS_TRAINING_SUCCESS   0x0000
#define XILINX_DISPLAYPORT_LPM_STATUS_LINK_VALID         0x0000
#define XILINX_DISPLAYPORT_LPM_STATUS_CHANGED            0x0010
#define XILINX_DISPLAYPORT_LPM_STATUS_RETRAIN            0x0011
#define XILINX_DISPLAYPORT_LPM_STATUS_NOT_CONNECTED      0x0012

#define XILINX_DISPLAYPORT_RX_IDLE                       0x0021
#define XILINX_DISPLAYPORT_RX_ACTIVE                     0x0022

#define XILINX_DISPLAYPORT_HPD_CONNECTION_TIMEOUT        (10 * SYSDEF_ONE_MILLISECOND)

#define RX_MONITOR_STATE_IDLE                               0x0000
#define RX_MONITOR_STATE_CR                                 0x0001
#define RX_MONITOR_STATE_CE                                 0x0002
#define RX_MONITOR_STATE_DONE                               0x0003

// Link configuration options
#define XILINX_DISPLAYPORT_LPM_USE_PREFERRED_MODE        0x01
#define XILINX_DISPLAYPORT_EDID_CORRUPT                  0xBADED1D

typedef enum
{
    HPD_STATE_DISCONNECTED,
    HPD_STATE_CONNECTED,
    HPD_STATE_INTERRUPT
} DisplayportHPDState;

typedef struct
{
    UINT8 lane_count;
    UINT8 link_rate;
    UINT8 efm, ssc, mlc;
    UINT32 training_settings;
    UINT32 video_mode_id;
    XILDPDPCDData sink_dpcd;
    XILDP_EDID_Info ei;
    UINT32 use_preferred_mode;
    UINT32 use_dpcd_caps;
    UINT32 failsafe;
    UINT32 video_mode_update;
    //UINT32 dtg_mode;
    UINT32 edid_disable;
    UINT8  bpc;
    UINT8  pattern;
    UINT8  pattern2;
    UINT8  pattern3;
    UINT8  pattern4;
    UINT8  comp_fmt;
    UINT8  dyn_range;
    UINT8  ycbcr_color;
    UINT8  sync_clock_mode;
    UINT8  aux_log_enable;
    UINT8  mst_enable;
    UINT8  no_of_mst_streams;
} XILDPMainLinkConfig;

typedef struct
{
    UINT8   misc0, pm;
    UINT32  enable_main_stream, enable_secondary_stream;
    UINT32  enable_tx_autodetection, enable_rx_autodetection, bypass_msa_filter;
    UINT32  reset_phy, reset_type; //blanking,  enable_vcap;
    char    *reset_phy_string;
    UINT32  preserve_linkrate;
    XILDPMainLinkConfig *mlconf;
    //char    key_press;
    //UINT32  enable_loopthrough_video;
    //UINT32  demo_mode;
    //TGIRect screen_rect;
    //TGIFrameBuffer  *screen;
} XILDPLinkPolicyMakerData;

UINT32      dplpmLinkPolicyMakerRunLoop(void);
void        dplpmDisplayHelp(void);
UINT32      dplpmReadDPCDStatus(void);
UINT32      dplpmReadStatus(void);
void        dplpmMainLinkEnable(UINT32 enable);
void        dplpmTogglePowerMode(void);
UINT32      dplpmVerifyLinkStatus(void);
UINT32      dplpmChangeLinkRate(void);
UINT32      dplpmChangeLaneCount(void);
UINT32      dplpmReadEDID(XILDP_EDID_Info *ei, UINT32 write_checksum);
UINT32      dplpmGetPreferredTiming(XILDP_EDID_Info *ei);
UINT32      dplpmCheckHPDStatus(void);
UINT32      dplpmConfigureLink(UINT32 failsafe, UINT32 test_enable, UINT8 read_edid);

UINT32      dplpmMaintainLink(UINT32 force_retrain, UINT8 read_edid);

UINT32      dplpmTrainLink(UINT32 training_settings, UINT32 preserve_linkrate, UINT32 force);

void        dplpmInitLinkPolicyMaker(UINT32 link_rate, UINT32 lane_count);

XILDPMainLinkConfig *dplpmGetMainLinkConfig(void);
XILDPLinkPolicyMakerData *dplpmGetLinkPolicyMakerData(void);


#endif /* __XILINX_DISPLAYPORT_LINK_POLICY_MAKER__ */
