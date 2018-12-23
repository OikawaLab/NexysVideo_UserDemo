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


#ifndef __MODE_TABLE__
#define __MODE_TABLE__

#include "xil_types.h"
#include "xil_ccc_app.h"

typedef struct {
    char *res_string;
    UINT32 pll_settings;
    UINT32 vsync_polarity;
    UINT32 vert_front_porch;
    UINT32 vert_sync_width;
    UINT32 vert_back_porch;
    UINT32 pixel_clk;
    UINT32 vert_refresh;
    UINT32 vert_res;
    UINT32 hsync_polarity;
    UINT32 horiz_front_porch;
    UINT32 horiz_sync_width;
    UINT32 horiz_back_porch;
    UINT32 horiz_res;
    UINT32 data_polarity;
    UINT32 invert_clock;
    UINT32 color_format;
} DisplayMode;

// Naming conventions:
//  VIDEO_MODE_<HRES>_<VRES>_<REFRESH_RATE>_<BIT_DEPTH>
//  TEST_MODE_<HRES>_<VRES>_<FORMAT_CODE>
//  RB indicates a reduced blanking mode

//typedef enum
//{
//    NO_VIDEO_MODE = 0,                         // ( 0) NO_VIDEO_MODE
//    TEST_MODE_144_90_32,                       // ( 1) TEST_MODE_144_90_32
//    TEST_MODE_360_225_32,                      // ( 2) TEST_MODE_360_225_32
//    VIDEO_MODE_QCIF,                           // ( 3) VIDEO_MODE_QCIF
//    VIDEO_MODE_CIF,                            // ( 4) VIDEO_MODE_CIF
//    VIDEO_MODE_4CIF,                           // ( 5) VIDEO_MODE_4CIF
//    VIDEO_MODE_480I,                           // ( 6) VIDEO_MODE_480I (NTSC)
//    VIDEO_MODE_480P,                           // ( 7) VIDEO_MODE_480P
//    VIDEO_MODE_720P,                           // ( 8) VIDEO_MODE_720P
//    VIDEO_MODE_RB_1080I,                       // ( 9) VIDEO_MODE_RB_1080I
//    VIDEO_MODE_RB_1080P,                       // (10) VIDEO_MODE_RB_1080P
//    VIDEO_MODE_640_480_60_32,                  // (11) VIDEO_MODE_640_480_60_32
//    VIDEO_MODE_800_600_60_32,                  // (12) VIDEO_MODE_800_600_60_32
//    VIDEO_MODE_1024_768_60_32,                 // (13) VIDEO_MODE_1024_768_60_32
//    VIDEO_MODE_1280_1024_60_32,                // (14) VIDEO_MODE_1280_1024_60_32
//    VIDEO_MODE_1280_1024_60_565,               // (15) VIDEO_MODE_1280_1024_60_565
//    VIDEO_MODE_1440_900_60_32,                 // (16) VIDEO_MODE_1440_900_60_32
//    VIDEO_MODE_RB_1440_900_60_32,              // (17) VIDEO_MODE_RB_1440_900_60_32
//    VIDEO_MODE_1680_1050_60_32,                // (18) VIDEO_MODE_1680_1050_60_32
//    VIDEO_MODE_RB_1680_1050_60_32,             // (19) VIDEO_MODE_RB_1680_1050_60_32
//    VIDEO_MODE_1920_1200_60_32,                // (20) VIDEO_MODE_1920_1200_60_32
//    VIDEO_MODE_RB_1920_1200_60_32,             // (21) VIDEO_MODE_RB_1920_1200_60_32
//    LAST_MODE = VIDEO_MODE_RB_1920_1200_60_32  // LAST_MODE need to be last element
//} MODE_ENUM;


typedef enum
{
    NO_VIDEO_MODE = 0,                         // ( 0) NO_VIDEO_MODE                 
    VIDEO_MODE_800_600_60_32,                  // (12) VIDEO_MODE_800_600_60_32      
    VIDEO_MODE_1024_768_60_32,                 // (13) VIDEO_MODE_1024_768_60_32     
    VIDEO_MODE_RB_1920_1200_60_32,             // (21) VIDEO_MODE_RB_1920_1200_60_32 
    LAST_MODE = VIDEO_MODE_RB_1920_1200_60_32  // LAST_MODE need to be last element
} MODE_ENUM;


#define MAX_DISPLAY_MODES (LAST_MODE+1)

#define PIXEL_CLOCK_RANGE_DIVISOR           20   // 20 = 5%
#define PIXEL_CLOCK_RATE_DIVISOR            1000 // Divide down actual clock rate for comparison
#define PIXEL_CLOCK_USE_DIVISOR             1

MODE_ENUM modetable_get_nearest_mode(UINT32 lines_per_frame);
void modetable_get_mode(UINT32 mode_id, DisplayMode *dm);
void modetable_dump_mode(UINT32 mode_id, DisplayMode *dm);
MODE_ENUM modetable_get_compatible_mode(UINT32 hres, UINT32 vres, UINT32 pixel_clk);
inline char *modetable_get_string(UINT32 mode);


#if RUN_DMT_MODES
static DMT_MODES dmt_modes[] =
{
   // mode_name, mode_id, hres, vres, pclk, scan, hpol, vpol, hfp, hsw, hbp, vfp, vsw, vbp
   {"640x480_60_P", 0x04, 640, 480, 25175, 0, 1, 1, 8, 96, 40, 2, 2, 25},//0
   {"800x600_60_P", 0x09, 800, 600, 40000, 0, 0, 0, 40, 128, 88, 1, 4, 23},//1
   {"848x480_60_P", 0x0E, 848, 480, 33750, 0, 0, 0, 16, 112, 112, 6, 8, 23},//2
   {"1024x768_60_P", 0x10, 1024, 768, 65000, 0, 1, 1, 24, 136, 160, 3, 6, 29},//3
   {"1280x768_60_P_RB", 0x16, 1280, 768, 68250, 0, 0, 1, 48, 32, 80, 3, 7, 12},//4
   {"1280x768_60_P", 0x17, 1280, 768, 79500, 0, 1, 0, 64, 128, 192, 3, 7, 20},//5
   {"1280x800_60_P_RB", 0x1B, 1280, 800, 71000, 0, 0, 1, 48, 32, 80, 3, 6, 14},//6
   {"1280x800_60_P", 0x1C, 1280, 800, 83500, 0, 1, 0, 72, 128, 200, 3, 6, 22},//7
   {"1280x960_60_P", 0x20, 1280, 960, 108000, 0, 0, 0, 96, 112, 312, 1, 3, 36},//8
   {"1280x1024_60_P", 0x23, 1280, 1024, 108000, 0, 1, 1, 48, 112, 248, 0, 3, 39},//9
   {"1360x768_60_P", 0x27, 1360, 768, 85500, 0, 0, 0, 64, 112, 256, 3, 6, 18},//a
   {"1400x1050_60_P_RB", 0x29, 1400, 1050, 101000, 0, 0, 1, 48, 32, 80, 3, 4, 23},//b
   {"1400x1050_60_P", 0x2A, 1400, 1050, 121750, 0, 1, 0, 88, 144, 232, 3, 4, 32},//c
   {"1440x900_60_P_RB", 0x2E, 1440, 900, 88750, 0, 0, 1, 48, 32, 80, 3, 6, 17},//d
   {"1440x900_60_P", 0x2F, 1440, 900, 106500, 0, 1, 0, 80, 152, 232, 3, 6, 25},//e
   {"1600x1200_60_P", 0x33, 1600, 1200, 162000, 0, 0, 0, 64, 192, 304, 1, 3, 46},//f
   {"1680x1050_60_P_RB", 0x39, 1680, 1050, 119000, 0, 0, 1, 48, 32, 80, 3, 6, 21},//10
   {"1680x1050_60_P", 0x3A, 1680, 1050, 146250, 0, 1, 0, 104, 176, 280, 3, 6, 30},//11
   {"1792x1344_60_P", 0x3E, 1792, 1344, 204750, 0, 1, 0, 128, 200, 328, 1, 3, 46},//12
   {"1856x1392_60_P", 0x41, 1856, 1392, 218250, 0, 1, 0, 96, 224, 352, 1, 3, 43},//13
   {"1920x1200_60_P_RB", 0x44, 1920, 1200, 154000, 0, 0, 1, 48, 32, 80, 3, 6, 26},//14
   {"1920x1200_60_P", 0x45, 1920, 1200, 193250, 0, 1, 0, 136, 200, 336, 3, 6, 36},//15
   {"1920x1440_60_P", 0x49, 1920, 1440, 234000, 0, 1, 0, 128, 208, 344, 1, 3, 56},//16
   {"2560x1600_60_P_RB", 0x4C, 2560, 1600, 268500, 0, 0, 1, 48, 32, 80, 3, 6, 37},//17
   {"2560x1600_60_P", 0x4D, 2560, 1600, 348500, 0, 1, 0, 192, 280, 472, 3, 6, 49},//18
   {"800x600_56_P", 0x08, 800, 600, 36000, 0, 0, 0, 24, 72, 128, 1, 2, 22},//19
   {"1600x1200_65_P", 0x34, 1600, 1200, 175500, 0, 0, 0, 64, 192, 304, 1, 3, 46},//1A
   {"1600x1200_70_P", 0x35, 1600, 1200, 189000, 0, 0, 0, 64, 192, 304, 1, 3, 46},//1b
   {"1024x768_70_P", 0x11, 1024, 768, 75000, 0, 1, 1, 24, 136, 144, 3, 6, 29},//1c
   {"640x480_72_P", 0x05, 640, 480, 31500, 0, 1, 1, 16, 40, 120, 1, 3, 20},//1d
   {"800x600_72_P", 0x0A, 800, 600, 50000, 0, 0, 0, 56, 120, 64, 37, 6, 23},//1e
   {"640x480_75_P", 0x06, 640, 480, 31500, 0, 1, 1, 16, 64, 120, 1, 3, 16},//1f
   {"800x600_75_P", 0x0B, 800, 600, 49500, 0, 0, 0, 16, 80, 160, 1, 3, 21},//20
   {"1024x768_75_P", 0x12, 1024, 768, 78750, 0, 0, 0, 16, 96, 176, 1, 3, 28},//21
   {"1152x864_75_P", 0x15, 1152, 864, 108000, 0, 0, 0, 64, 128, 256, 1, 3, 32},//22
   {"1280x768_75_P", 0x18, 1280, 768, 102250, 0, 1, 0, 80, 128, 208, 3, 7, 27},//23
   {"1280x800_75_P", 0x1D, 1280, 800, 106500, 0, 1, 0, 80, 128, 208, 3, 6, 29},//24
   {"1280x1024_75_P", 0x24, 1280, 1024, 135000, 0, 0, 0, 16, 144, 248, 1, 3, 38},//25
   {"1400x1050_75_P", 0x2B, 1400, 1050, 156000, 0, 1, 0, 104, 144, 248, 3, 4, 42},//26
   {"1440x900_75_P", 0x30, 1440, 900, 136750, 0, 1, 0, 96, 152, 31, 3, 6, 33},//27
   {"1600x1200_75_P", 0x36, 1600, 1200, 202500, 0, 0, 0, 64, 192, 304, 1, 3, 46},//28
   {"1680x1050_75_P", 0x3B, 1680, 1050, 187000, 0, 1, 0, 120, 176, 37, 3, 6, 40},//29
   {"1792x1344_75_P", 0x3F, 1792, 1344, 261000, 0, 1, 0, 96, 216, 352, 1, 3, 69},//2a
   {"1856x1392_75_P", 0x42, 1856, 1392, 288000, 0, 1, 0, 128, 224, 352, 1, 3, 104},//2b
   {"1920x1200_75_P", 0x46, 1920, 1200, 245250, 0, 1, 0, 136, 208, 344, 3, 6, 46},//2c
   {"1920x1440_75_P", 0x4A, 1920, 1440, 297000, 0, 1, 0, 144, 224, 352, 1, 3, 56},//2d
   {"2560x1600_75_P", 0x4E, 2560, 1600, 443250, 0, 1, 0, 208, 280, 488, 3, 6, 63},//2e
   {"640x350_85_P", 0x01, 640, 350, 31500, 0, 0, 1, 32, 64, 96, 32, 3, 60},//2f
   {"640x400_85_P", 0x02, 640, 400, 31500, 0, 1, 0, 32, 64, 96, 1, 3, 41},//30
   {"720x400_85_P", 0x03, 720, 400, 35500, 0, 1, 0, 36, 72, 108, 1, 3, 42},//31
   {"640x480_85_P", 0x07, 640, 480, 36000, 0, 1, 1, 56, 56, 80, 1, 3, 25},//32
   {"800x600_85_P", 0x0C, 800, 600, 56250, 0, 0, 0, 32, 64, 152, 1, 3, 27},//33
   {"1024x768_85_P", 0x13, 1024, 768, 94500, 0, 0, 0, 48, 96, 208, 1, 3, 36},//34
   {"1280x768_85_P", 0x19, 1280, 768, 117500, 0, 1, 0, 80, 136, 216, 3, 7, 31},//35
   {"1280x800_85_P", 0x1E, 1280, 800, 122500, 0, 1, 0, 80, 136, 216, 3, 6, 34},//36
   {"1280x960_85_P", 0x21, 1280, 960, 148500, 0, 0, 0, 64, 160, 224, 1, 3, 47},//37
   {"1280x1024_85_P", 0x25, 1280, 1024, 157500, 0, 0, 0, 64, 160, 224, 1, 3, 44},//38
   {"1400x1050_85_P", 0x2C, 1400, 1050, 179500, 0, 1, 0, 104, 152, 256, 3, 4, 48},//39
   {"1440x900_85_P", 0x31, 1440, 900, 157000, 0, 1, 0, 104, 152, 32, 3, 6, 39},//3a
   {"1600x1200_85_P", 0x37, 1600, 1200, 229500, 0, 0, 0, 64, 192, 304, 1, 3, 46},//3b
   {"1680x1050_85_P", 0x3C, 1680, 1050, 214750, 0, 1, 0, 128, 176, 304, 3, 6, 46},//3c
   {"1920x1200_85_P", 0x47, 1920, 1200, 281250, 0, 1, 0, 144, 208, 352, 3, 6, 53},//3d
   {"2560x1600_85_P", 0x4F, 2560, 1600, 505250, 0, 1, 0, 208, 280, 488, 3, 6, 73},//3e
   {"800x600_120_P_RB", 0x0D, 800, 600, 73250, 0, 0, 1, 48, 32, 80, 3, 4, 29},//3f
   {"1024x768_120_P_RB", 0x14, 1024, 768, 115500, 0, 0, 1, 48, 32, 80, 3, 4, 38},//40
   {"1280x768_120_P_RB", 0x1A, 1280, 768, 140250, 0, 0, 1, 48, 32, 80, 3, 7, 35},//41
   {"1280x800_120_P_RB", 0x1F, 1280, 800, 146250, 0, 0, 1, 48, 32, 80, 3, 6, 38},//42
   {"1280x960_120_P_RB", 0x22, 1280, 960, 175500, 0, 0, 1, 48, 32, 80, 3, 4, 50},//43
   {"1280x1024_120_P_RB", 0x26, 1280, 1024, 187250, 0, 0, 1, 48, 32, 80, 3, 7, 50},//44
   {"1360x768_120_P_RB", 0x28, 1360, 768, 148250, 0, 0, 1, 48, 32, 80, 3, 5, 37},//45
   {"1400x1050_120_P_RB", 0x2D, 1400, 1050, 208000, 0, 0, 1, 48, 32, 80, 3, 4, 55},//46
   {"1440x900_120_P_RB", 0x32, 1440, 900, 182750, 0, 0, 1, 48, 32, 80, 3, 6, 44},//47
   {"1600x1200_120_P_RB", 0x38, 1600, 1200, 268250, 0, 0, 1, 48, 32, 80, 3, 4, 64},//48
   {"1680x1050_120_P_RB", 0x3D, 1680, 1050, 245500, 0, 0, 1, 48, 32, 80, 3, 6, 53},//49
   {"1792x1344_120_P_RB", 0x40, 1792, 1344, 333250, 0, 0, 1, 48, 32, 80, 3, 4, 72},//4a
   {"1856x1392_120_P_RB", 0x43, 1856, 1392, 356500, 0, 0, 1, 48, 32, 80, 3, 4, 75},//4b
   {"1920x1200_120_P_RB", 0x48, 1920, 1200, 317000, 0, 0, 1, 48, 32, 80, 3, 6, 62},//4c
   {"1920x1440_120_P_RB", 0x4B, 1920, 1440, 380500, 0, 0, 1, 48, 32, 80, 3, 4, 78},//4d
   {"2560x1600_120_P_RB", 0x50, 2560, 1600, 552750, 0, 0, 1, 48, 32, 80, 3, 6, 85},//4e
   {"1366x768_60_P", 0x00, 1366, 768, 72000, 0, 0, 0, 14, 56, 64, 1, 3, 28},//4f
   {"1080p@60", 0x00, 1920, 1080, 148500, 0, 1, 1, 88, 44, 148, 4, 5, 36}//0x50
   // mode_name, mode_id, hres, vres, pclk, scan, hpol, vpol, hfp, hsw, hbp, vfp, vsw, vbp
};
#else
static DMT_MODES dmt_modes[] =
{
  // mode_name, mode_id, hres, vres, pclk, scan, hpol, vpol, hfp, hsw, hbp, vfp, vsw, vbp
  {"640x480_60_P", 0x04, 640, 480, 25175, 0, 1, 1, 8, 96, 40, 2, 2, 25},
  {"800x600_60_P", 0x09, 800, 600, 40000, 0, 0, 0, 40, 128, 88, 1, 4, 23},
  {"1024x768_60_P", 0x10, 1024, 768, 65000, 0, 1, 1, 24, 136, 160, 3, 6, 29},
  {"1280x768_60_P", 0x17, 1280, 768, 79500, 0, 1, 0, 64, 128, 192, 3, 7, 20},
  {"1280x800_60_P", 0x1C, 1280, 800, 83500, 0, 1, 0, 72, 128, 200, 3, 6, 22},
  {"1280x1024_60_P", 0x23, 1280, 1024, 108000, 0, 0, 0, 48, 112, 248, 1, 3, 38},
  {"1440x900_60_P", 0x2F, 1440, 900, 106500, 0, 1, 0, 80, 152, 232, 3, 6, 25},
  {"1600x1200_60_P", 0x33, 1600, 1200, 162000, 0, 0, 0, 64, 192, 304, 1, 3, 46},
  {"1680x1050_60_P", 0x3A, 1680, 1050, 146250, 0, 1, 0, 104, 176, 280, 3, 6, 30},
  {"1920x1200_60_P_RB", 0x44, 1920, 1200, 154000, 0, 0, 1, 48, 32, 80, 3, 6, 26},
  {"1920x1200_60_P", 0x45, 1920, 1200, 193250, 0, 1, 0, 136, 200, 336, 3, 6, 36},
};
#endif
#endif


