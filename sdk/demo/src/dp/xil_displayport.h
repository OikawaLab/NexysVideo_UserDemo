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

#ifndef __XILINX_DISPLAYPORT_DRV__
#define __XILINX_DISPLAYPORT_DRV__

#include "displayport_tx_drv.h"
#include "xbasic_types.h"

//#include "xil_graphics.h"
//#include "mode_table.h"

//  AUX CH DPCD Info
#define XILINX_DISPLAYPORT_DPCD_REVISION                         0x0000 // DPCD Revision Number                                                         
#define XILINX_DISPLAYPORT_DPCD_MAX_LINK_RATE                    0x0001 // Max Link Rate                                                                
#define XILINX_DISPLAYPORT_DPCD_MAX_LANE_COUNT                   0x0002 // Max Lane Count                                                               
#define XILINX_DISPLAYPORT_DPCD_MAX_DOWNSPREAD                   0x0003 // Max Downspread                                                               
#define XILINX_DISPLAYPORT_DPCD_NUM_RX_PORTS                     0x0004 // Number of Receive Ports                                                      
#define XILINX_DISPLAYPORT_DPCD_DOWNSTREAM_PORTS_PRESENT         0x0005 // Downstream port present                                                      
#define XILINX_DISPLAYPORT_DPCD_MAIN_LINK_CODING                 0x0006 // Main link channel coding                                                     
#define XILINX_DISPLAYPORT_DPCD_NUM_DOWNSTREAM_PORTS             0x0007 // Downstream port count                                                        
#define XILINX_DISPLAYPORT_DPCD_RX_PORT0_CAPS_0                  0x0008 // Receive Port 0 Capabilities 0                                                
#define XILINX_DISPLAYPORT_DPCD_RX_PORT0_CAPS_1                  0x0009 // Receive Port 0 Capabilities 1                                                
#define XILINX_DISPLAYPORT_DPCD_RX_PORT1_CAPS_0                  0x000A // Receive Port 1 Capabilities 0                                                
#define XILINX_DISPLAYPORT_DPCD_RX_PORT1_CAPS_1                  0x000B // Receive Port 1 Capabilities 1
#define XILINX_DISPLAYPORT_DPCD_DOWNSTREAM_PORT_CAPS             0x0080 // Downstream Port 0 - 15 Capabilities
#define XILINX_DISPLAYPORT_DPCD_NUM_DOWNSTREAM_PORT_CAPS         16
#define XILINX_DISPLAYPORT_DPCD_GUID                             0x0030 // 0x30-0x3F
                                                                    
#define XILINX_DISPLAYPORT_DPCD_LINK_BANDWIDTH_SET               0x0100 // Link Bandwidth Set                                                           
#define XILINX_DISPLAYPORT_DPCD_LANE_COUNT_SET                   0x0101 // Lane Count Set                                                               
#define XILINX_DISPLAYPORT_DPCD_TRAINING_PATTERN_SET             0x0102 // Training Pattern Set                                                         
#define XILINX_DISPLAYPORT_DPCD_TRAINING_LANE0_SET               0x0103 // Training Adjustment Lane 0 Set                                                          
#define XILINX_DISPLAYPORT_DPCD_TRAINING_LANE1_SET               0x0104 // Training Adjustment Lane 1 Set                                                          
#define XILINX_DISPLAYPORT_DPCD_TRAINING_LANE2_SET               0x0105 // Training Adjustment Lane 2 Set                                                          
#define XILINX_DISPLAYPORT_DPCD_TRAINING_LANE3_SET               0x0106 // Training Adjustment Lane 3 Set                                                          
#define XILINX_DISPLAYPORT_DPCD_DOWNSPREAD_CONTROL               0x0107 // Downspread Control                                                           
#define XILINX_DISPLAYPORT_DPCD_MAIN_LINK_CODING_SET             0x0108 // Main Link Channel Coding Set

#define XILINX_DISPLAYPORT_DPCD_SINK_COUNT                       0x0200 // Sink Count                                                                   
#define XILINX_DISPLAYPORT_DPCD_DEVICE_SERVICE_IRQ               0x0201 // Device Service IRQ Vector                                                    
#define XILINX_DISPLAYPORT_DPCD_STATUS_LANE_0_1                  0x0202 // Lane 0 & 1 Status                                                            
#define XILINX_DISPLAYPORT_DPCD_STATUS_LANE_2_3                  0x0203 // Lane 2 & 3 Status                                                            
#define XILINX_DISPLAYPORT_DPCD_LANE_ALIGNMENT_STATUS_UPDATED    0x0204 // Lane Alignment Status Updated                                                
#define XILINX_DISPLAYPORT_DPCD_SINK_STATUS                      0x0205 // Sink Status                                                                  
#define XILINX_DISPLAYPORT_DPCD_ADJUST_REQ_LANE_0_1              0x0206 // Adjust Request Lane 0 & 1                                                    
#define XILINX_DISPLAYPORT_DPCD_ADJUST_REQ_LANE_2_3              0x0207 // Adjust Request Lane 2 & 3                                                    
#define XILINX_DISPLAYPORT_DPCD_TRAINING_SCORE_LANE_0            0x0208 // Training Score Lane 0                                                        
#define XILINX_DISPLAYPORT_DPCD_TRAINING_SCORE_LANE_1            0x0209 // Training Score Lane 1                                                        
#define XILINX_DISPLAYPORT_DPCD_TRAINING_SCORE_LANE_2            0x020A // Training Score Lane 2                                                        
#define XILINX_DISPLAYPORT_DPCD_TRAINING_SCORE_LANE_3            0x020B // Training Score Lane 3                                                        
                                                                    
#define XILINX_DISPLAYPORT_DPCD_SYMBOL_ERROR_COUNT_LANE_0        0x0210 // Symbol Error Count Lane 0                                                    
#define XILINX_DISPLAYPORT_DPCD_SYMBOL_ERROR_COUNT_LANE_1        0x0212 // Symbol Error Count Lane 1                                                    
#define XILINX_DISPLAYPORT_DPCD_SYMBOL_ERROR_COUNT_LANE_2        0x0214 // Symbol Error Count Lane 2                                                    
#define XILINX_DISPLAYPORT_DPCD_SYMBOL_ERROR_COUNT_LANE_3        0x0216 // Symbol Error Count Lane 3                                                    
#define XILINX_DISPLAYPORT_DPCD_SYMBOL_ERROR_COUNT_LANE_3        0x0216 // Symbol Error Count Lane 3                                                    

#define XILINX_DISPLAYPORT_DPCD_TEST_REQUEST                     0x0218 // Test Request
#define XILINX_DISPLAYPORT_DPCD_TEST_LINK_RATE                   0x0219 // Test Link Rate
#define XILINX_DISPLAYPORT_DPCD_TEST_LANE_COUNT                  0x0220 // Test Lane Count
#define XILINX_DISPLAYPORT_DPCD_TEST_PATTERN                     0x0221 // Test Pattern
#define XILINX_DISPLAYPORT_DPCD_TEST_RESPONSE                    0x0260 // Test Response
#define XILINX_DISPLAYPORT_DPCD_TEST_EDID_CHECKSUM               0x0261 // Test EDID Checksum

#define XILINX_DISPLAYPORT_DPCD_PAYLOAD_TABLE_UPDATE_STATUS      0x02C0
#define XILINX_DISPLAYPORT_DPCD_VC_PAYLOAD_ID_SLOT               0x02C1 // start address

// Source Device-Specific Field 0x0303 - 0x003FF : Reserved for vendor-specific usage    
#define XILINX_DISPLAYPORT_DPCD_SOURCE_IEEE_OUI_0                0x0300 // Source IEEE OUI 7:0                                                          
#define XILINX_DISPLAYPORT_DPCD_SOURCE_IEEE_OUI_1                0x0301 // Source IEEE OUI 15:8                                                         
#define XILINX_DISPLAYPORT_DPCD_SOURCE_IEEE_OUI_2                0x0302 // Source IEEE OUI 23:16                                                        

// Sink Device-Specific Field    0x0403 - 0x004FF : Reserved for vendor-specific usage                                      
#define XILINX_DISPLAYPORT_DPCD_SINK_IEEE_OUT_0                  0x0400 // Sink IEEE OUI 7:0                                                            
#define XILINX_DISPLAYPORT_DPCD_SINK_IEEE_OUT_1                  0x0401 // Sink IEEE OUI 15:8                                                           
#define XILINX_DISPLAYPORT_DPCD_SINK_IEEE_OUT_2                  0x0402 // Sink IEEE OUI 23:16                                                          

// Branch Device-Specific Field  0x0503 - 0x005FF : Reserved for vendor-specific usage                                                   
#define XILINX_DISPLAYPORT_DPCD_BRANCH_DEVICE_IEEE_OUI_0         0x0500 // Branch Device IEEE OUI 7:0                                                   
#define XILINX_DISPLAYPORT_DPCD_BRANCH_DEVICE_IEEE_OUI_1         0x0501 // Branch Device IEEE OUI 15:8                                                  
#define XILINX_DISPLAYPORT_DPCD_BRANCH_DEVICE_IEEE_OUI_2         0x0502 // Branch Device IEEE OUI 23:16                                                 
#define XILINX_DISPLAYPORT_DPCD_SET_POWER                        0x0600 // Set Power

// Side Band Buffers
#define XILINX_DISPLAYPORT_DPCD_DOWN_REQ                         0x01000 // Down Request: 0x01000 - 0x011FFh
#define XILINX_DISPLAYPORT_DPCD_UP_REP                           0x01200 // Down Request: 0x01200 - 0x013FFh
#define XILINX_DISPLAYPORT_DPCD_DOWN_REP                         0x01400 // Down Request: 0x01400 - 0x015FFh
#define XILINX_DISPLAYPORT_DPCD_UP_REQ                           0x01600 // Down Request: 0x01600 - 0x017FFh

#define XILINX_DISPLAYPORT_DPCD_DEVICE_SERVICE_IRQ_ESI0          0x2003 // Device Service IRQ Vector - ESI

// Lane status register constants
#define XILINX_DISPLAYPORT_LANE_0_STATUS_CLK_REC_DONE            0x01
#define XILINX_DISPLAYPORT_LANE_0_STATUS_CHAN_EQ_DONE            0x02
#define XILINX_DISPLAYPORT_LANE_0_STATUS_SYM_LOCK_DONE           0x04
#define XILINX_DISPLAYPORT_LANE_1_STATUS_CLK_REC_DONE            0x10
#define XILINX_DISPLAYPORT_LANE_1_STATUS_CHAN_EQ_DONE            0x20
#define XILINX_DISPLAYPORT_LANE_1_STATUS_SYM_LOCK_DONE           0x40
#define XILINX_DISPLAYPORT_LANE_2_STATUS_CLK_REC_DONE            XILINX_DISPLAYPORT_LANE_0_STATUS_CLK_REC_DONE      
#define XILINX_DISPLAYPORT_LANE_2_STATUS_CHAN_EQ_DONE            XILINX_DISPLAYPORT_LANE_0_STATUS_CHAN_EQ_DONE 
#define XILINX_DISPLAYPORT_LANE_2_STATUS_SYM_LOCK_DONE           XILINX_DISPLAYPORT_LANE_0_STATUS_SYM_LOCK_DONE
#define XILINX_DISPLAYPORT_LANE_3_STATUS_CLK_REC_DONE            XILINX_DISPLAYPORT_LANE_1_STATUS_CLK_REC_DONE 
#define XILINX_DISPLAYPORT_LANE_3_STATUS_CHAN_EQ_DONE            XILINX_DISPLAYPORT_LANE_1_STATUS_CHAN_EQ_DONE 
#define XILINX_DISPLAYPORT_LANE_3_STATUS_SYM_LOCK_DONE           XILINX_DISPLAYPORT_LANE_1_STATUS_SYM_LOCK_DONE
#define XILINX_DISPLAYPORT_LANE_ALIGNMENT_DONE                   0x01

// Link training constants
#define MAX_LINK_TRAINING_SEQUENCE_ITERATIONS                       5
#define XILINX_DISPLAYPORT_CLOCK_REC_TIMEOUT                     120 //* SYSDEF_ONE_MICROSECOND
#define XILINX_DISPLAYPORT_CHAN_EQ_TIMEOUT                       400 //* SYSDEF_ONE_MICROSECOND
#define XILINX_DISPLAYPORT_CR_FAILED                             0xDEADD00D
#define XILINX_DISPLAYPORT_MAX_DEFER_COUNT                       50
#define XILINX_DISPLAYPORT_MAX_TIMEOUT_COUNT                     50

#define XILINX_DISPLAYPORT_CONFIG_VALID                          0x00000000
#define XILINX_DISPLAYPORT_CONFIG_INVALID_LINK_RATE              0x0000EEAA
#define XILINX_DISPLAYPORT_CONFIG_INVALID_LANE_COUNT             0x0000EEBB
#define XILINX_DISPLAYPORT_CONFIG_HPD_DEASSERTED                 0x0000EECC

// Link training state constants
#define XILINX_DISPLAYPORT_TS_CLOCK_REC                          0x01
#define XILINX_DISPLAYPORT_TS_CHANNEL_EQ                         0x02
#define XILINX_DISPLAYPORT_TS_ADJUST_SPD                         0x04
#define XILINX_DISPLAYPORT_TS_ADJUST_LANES                       0x08
#define XILINX_DISPLAYPORT_TS_UPDATE_STATUS                      0x10

typedef struct
{
    UINT8 dpcd_rev;
    UINT8 max_lane_count;
    UINT8 max_link_speed;
    UINT8 downstream_port_caps[XILINX_DISPLAYPORT_DPCD_NUM_DOWNSTREAM_PORT_CAPS];
    char *downstream_port_types[XILINX_DISPLAYPORT_DPCD_NUM_DOWNSTREAM_PORT_CAPS];
    char *rev_string, *link_rate_string, *port_type_string;
    UINT32 num_rcv_ports, num_downstream_ports;
    Xboolean format_conversion_support, oui_support, ansi_8B10_support, enhanced_framing_support;
    Xboolean downspread_support, require_aux_handshake, rx0_has_edid, rx0_use_prev;
    UINT8 rx0_buffer_size;
} XILDPDPCDData;

typedef struct
{
    UINT32 h_clk_total,  v_clk_total; 
    UINT32 h_sync_width, v_sync_width;
    UINT32 h_resolution, v_resolution;
    UINT32 h_sync_polarity, v_sync_polarity;
    UINT32 h_start, v_start;   
    UINT32 misc0, misc1;       
    UINT32 m_vid, n_vid;       
    UINT32 transfer_unit_size; 
    UINT32 user_pixel_width;   
    UINT32 data_per_lane;      
    UINT32 interlaced;         
    UINT32 bytes_per_pixel;    
} XILDPMainStreamAttributes;

// Initialization
       UINT32 xildpInitDisplayport(void);
// AUX CH functions
       UINT32  xildpAUXRead(UINT32 address, UINT32 byte_count, void* data);
       UINT32  xildpAUXWrite(UINT32 address, UINT32 byte_count, void* data);
       UINT32  xildpAUXSubmitCommand(XILDPAUXTransaction *request);
// I2C functions
       UINT32 xildpI2CRead(UINT32 device_address, UINT32 register_address, UINT32 byte_count, UINT8* data_buffer, UINT8 first_req, UINT8 last_req, UINT8 exit_when_defer);
       UINT32 xildpI2CWrite(UINT32 device_address, UINT32 register_address, UINT32 byte_count, UINT8* data_buffer);
// Utility functions
         void xildpWaituS(UINT32 us);
       UINT32 xildpReadEDID(UINT8 *edid_data);
         char *xildpGetDPCDRegName(UINT32 address);
inline UINT32 xildpGetHPDStatus(void);
       UINT32 xildpGetCoreID(void);
// TX Parameter setting functions
       UINT32 xildpSetLinkRate(UINT8 link_rate);
       UINT32 xildpSetLaneCount(UINT32 lane_count);
       UINT32 xildpSetPowerState(UINT32 pwr_state);
       UINT32 xildpSetDownspread(UINT8 downspread_on);
       UINT32 xildpSetLinkQualityPattern(UINT8 pattern);
       UINT32 xildpSetTrainingPattern(UINT32 pattern);
       UINT32 xildpSetEnhancedFramingMode(UINT32 efm_enable);
       UINT32 xildpSetMainLinkChannelCoding(UINT8 enable_8b10b);
         void xildpSetMSAValues(XILDPMainStreamAttributes *msa_values);

       UINT32 xildpGetSinkCaps(XILDPDPCDData *dpcd_data);

         void xildpDumpDPCD(void);
// Training parameter adjustment functions
         void xildpPresetVSwing(UINT32 fixed_vswing);
         void xildpPresetPreemphasis(UINT32 fixed_preemp);
       UINT32 xildpDPCDVSwingForValue(UINT32 tx_vswing);
       UINT32 xildpDPCDPreemphasisForValue(UINT32 tx_preemp, UINT32 current_vswing);
       UINT32 xildpSourcePreemphasisForValue(UINT32 dpcd_preemp, UINT32 current_vswing);
       UINT32 xildpSourceVSwingForValue(UINT32 dpcd_vswing);
// Link training functions
       UINT32 xildpSelectTrainingSettings(UINT32 training_settings);
       UINT32 xildpRunChannelEqualization(void);
       UINT32 xildpRunClockRecovery(void);
       UINT32 xildpRunClockRecoveryLoop(UINT32 max_iterations, UINT32 adaptive);
       UINT32 xildpRunChannelEqualizationLoop(UINT32 max_iterations, UINT32 adaptive, UINT8 link_rate);
       UINT32 xildpRunTrainingLoop(UINT32 training_settings, UINT32 adaptive, UINT32 preserve_linkrate);
       UINT32 xildpRunTrainingAdjustment(void);
       UINT32 xildpVerifyTrainingConfiguration(void);
       UINT32 xildpSetTrainingValues(void);
       UINT32 xildpUpdateStatus(void);
         void xildpDisplayTXRegisters(UINT32 show_msa_values);
         void xildpDisplayMSAValuesTX(void);
         void xildpDisplayPatGen (void);

         void xildpResetPHYTX(UINT32 reset_type);
         void xildpClearMSAValues(void);
         char getDPCDAddrName(UINT32 addr);

#endif /* __XILINX_DISPLAYPORT_DRV__ */
