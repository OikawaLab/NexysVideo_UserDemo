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

#ifndef __XILINX_EDID_DRIVER__
#define __XILINX_EDID_DRIVER__

#include "xbasic_types.h"
#include "displayport_defs.h"

#define EDID_DTD_UNKNOWN                0xAA
#define EDID_DTD_DETAILED_TIMING        0xF0
#define EDID_DTD_SERIAL_NUM             0xFF
#define EDID_DTD_ASCII_STRING           0xFE
#define EDID_DTD_RANGE_LIMITS           0xFD
#define EDID_DTD_NAME                   0xFC
#define EDID_DTD_COLOR_POINT            0xFB
#define EDID_DTD_STD_TIMING             0xFA
#define EDID_DTD_UNDEFINED              0xF9
#define EDID_DTD_MFG_DEFINED            0xF8

#define XIL_EDID_MAX_DTD_BLOCKS          0x04
#define XIL_EDID_MAX_STD_TIMING_MODES    7

typedef struct
{
    UINT32 h_res;
    UINT32 v_res;
    UINT8  v_freq;
    UINT8  aspect[2];
    char mode_string[32];
} EDID_Std_Timing_Mode;

typedef struct
{
    UINT8 v_freq_min, v_freq_max;   // in Hz
    UINT8 h_freq_min, h_freq_max;   // in Hz
    UINT8 pxlclk;                   // In MHz, multiply by 10 for actual
    UINT16 gtf_toggle;              // 000A = bytes 59-63, 0200 bytes 67-71
    UINT8 h_freq_start;             // In kHz, multiply by 2 for actual

//  64�65: Secondary GTF toggle
//    If encoded value is 000A, bytes 59-63 are used.  If encoded value is 0200, bytes 67�71 are used.
//  66: Start horizontal frequency (in kHz).  Multiply by 2 for actual value.
//  67: C. Divide by 2 for actual value.
//  68-69: M (little endian).
//  70: K
//  71: J. Divide by 2 for actual value.
}EDID_Range_Limit;

typedef struct
{
    UINT16 red_x, red_y;
    UINT16 green_x, green_y;
    UINT16 blue_x, blue_y;
    UINT16 white_x, white_y;
} EDID_Chromaticity;

typedef struct
{
    UINT8 dtd_type;
    UINT16 pixel_clock;
    UINT16 h_active, h_blanking;
    UINT16 v_active, v_blanking;
    UINT16 h_sync_pulse_offset, h_sync_pulse_width;
    UINT16 v_sync_pulse_offset, v_sync_pulse_width;
    UINT16 h_size, v_size;
    UINT8 size_high;
    UINT8 h_border, v_border;
    Xboolean  is_interlaced, vsync_pol, hsync_pol, serrate, sync_on_RGB, is_stereo;
    UINT8 flags;
    char *stereo_mode, *sync_type;
    UINT8 v_rate_max, v_rate_min, h_rate_max, h_rate_min, sec_timing;;
    UINT16 pxlclk_max;
    UINT8 gtf_fq_start, gtf_c, gtf_k, gtf_j;
    UINT16 gtf_m;
    char  block_string[14];
    EDID_Std_Timing_Mode std_modes[XIL_EDID_MAX_STD_TIMING_MODES];
    UINT8 std_mode_count;
    EDID_Chromaticity chromaticity_table;
} EDID_Detailed_Timing_Block;

typedef struct
{
    UINT16 vendor_id, product_id;
    UINT32 serial_num;
    UINT8 version;             // (0x12) Must be 1
    UINT8 revision;             // (0x13) Must be non-zero and less than current EEDID revision
    UINT8 max_hsize;            // (0x15) Non-zero for all but projectors
    UINT8 max_vsize;            // (0x16) Non-zero for all but projectors
    UINT8 dtc_gamma;            // (0x17) Display Transfer Characteristic (Gamma)
    char mfg_date[16];
    char edid_ver_str[20];
    Xboolean isDigitalDisplay;
    Xboolean dpms_active_off, dpms_suspend, dpms_standby;
    Xboolean supportsEDIDRev3;
    Xboolean supportsGTF;
    Xboolean has_sRGB_chromaticity;
    EDID_Chromaticity chromaticity_table;
    char *established_modes[17];
    EDID_Std_Timing_Mode timing_modes[8];
    EDID_Detailed_Timing_Block dtd_blocks[XIL_EDID_MAX_DTD_BLOCKS];
    UINT8 extension_flag;       // (0x7E) 
    UINT8 checksum;             // (0x7F)
    //UINT8 raw_data[128];
} XILDP_EDID_Info;

extern UINT8 edid_test_data_1[], edid_test_data_2[], edid_test_data_3[];

// UINT32 parse_edid(XILDP_EDID_Info *edid_info, UINT8 *edid_data);
// UINT32 parse_std_timing(UINT8 *std_timings, EDID_Std_Timing_Mode *mode_array);
// void   parse_dtd_blocks(XILDP_EDID_Info* edid_info, UINT8 *blk_data);
// UINT8  edid_compute_checksum(UINT8 *edid_data);
// UINT32 edid_verify_checksum(UINT8 *edid_data);
// void   edid_dtd_decode_flags(EDID_Detailed_Timing_Block *blk);
// void   edid_decode_chromaticity(EDID_Chromaticity *chr, UINT8 *chr_data);

// void   print_edid(XILDP_EDID_Info *edid);
// void   print_dtd_block(EDID_Detailed_Timing_Block *dtd_block);
// void   print_std_timing(EDID_Std_Timing_Mode *std_mode_array);
// void   print_established_timing(char **mode_array);
// void   print_chromaticity_table(EDID_Chromaticity *ct);
// void   print_detailed_timing(EDID_Detailed_Timing_Block *dtd_block);

#endif /* __XILINX_EDID_DRIVER__ */
