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


#include "xil_ccc_app.h"
#include "displayport_lpm.h"
#include "xlib_string.h"
#include "xil_types.h"
#include "xbasic_types.h"
#include "xil_displayport.h"
#include "mode_table.h"
#include "stdio.h"
//#include "dp_mst_sideband.h"
//#include "xtmrctr.h"

XILCCCAppControl *xilcccGetAppControl(void)
{
    static XILCCCAppControl xilccc_app_control;
    return &xilccc_app_control;
}
/*
 *  Function: Initializes the default configuration parameters for DisplayPort
 *
 *  Parameters:
 *
 *
 *  Returns:
 *      (UINT32) Status code; zero indicates success, non-zero indicates an error condition
 *
 *  Example:
 *
 *
 */
UINT32 xilcccAppInit(void)
{
    UINT32 status = 0;
    XILCCCAppControl *app_ctrl = xilcccGetAppControl();

    // Set up defaults for the link policy maker
    dplpmInitLinkPolicyMaker(XILINX_DISPLAYPORT_LINK_RATE_2_7GBPS, 2);

    app_ctrl->cmd_key = 0x00;
    app_ctrl->enable_vcap = 0;
    app_ctrl->enable_single_frame_capture = 0;

    return status;
}

/*
 *  Function: Main execution loop - for training and command processor
 *
 *  Parameters:
 *
 *
 *  Returns:
 *      (UINT32) Status code; zero indicates success, non-zero indicates an error condition
 *
 *  Example:
 *
 *
 */
UINT32 xilcccAppRunLoop(void)
{
    UINT32 status = 0, done = 0;
    XILCCCAppControl *app_ctrl = xilcccGetAppControl();

//    xilcccDisplayHelp();
//    dbg_printf("\033[s");//->saves cursor position

    // Main execution loop
    while(!done)
    {
        // Link Policy Maker loop - hot plug detect and link training
        dplpmLinkPolicyMakerRunLoop();

#ifdef LLC_TEST_RUN
        app_ctrl->cmd_key = xil_getc(0xff);
#else
        app_ctrl->cmd_key = xil_getc(0x100);
#endif

        //dbg_printf("\033[24H\033[J");->puts the cursor at 25th line
        dbg_printf(">\b");
        if(app_ctrl->cmd_key != 0){ //If character is received from UART console
            done = xilcccCommandProcessor(app_ctrl->cmd_key);
        }

    }
    return status;
}

/*
 *  Function:
 *
 *  Parameters:
 *
 *  Returns:
 *      (UINT32) Status code; zero indicates success, non-zero indicates an error condition
 *
 *  Example:
 *
 *
 */
/*UINT32 xilcccCommandProcessor(char command_key)
{
    XILDPLinkPolicyMakerData *lpmd = dplpmGetLinkPolicyMakerData();
    XILDPMainLinkConfig *mlconf = dplpmGetMainLinkConfig();
    UINT32 status = 0, done = 0;
    UINT32 addr;
    UINT32 rdata;
    UINT32 ii=0;
    UINT8 aux_data[4];
    u8 term_key, command;
    UINT32 red_pixel0_crc;
    UINT32 green_pixel0_crc;
    UINT32 blue_pixel0_crc;
    UINT32 red_pixel1_crc;
    UINT32 green_pixel1_crc;
    UINT32 blue_pixel1_crc;
//    dbg_printf("\033[u\033[J");//->recovers cursor position
//    dbg_printf("\033[s");//->saves cursor position

#if 0
    static UINT8 color_mode=0;
    UINT8 comp_format = 0;
    UINT8 dyn_range = 0;
    UINT8 ycbcr_color = 0;
    UINT8 bpc = 0;
#endif

    switch(command_key)
    {
//        case 'a':
//            if (lpmd->enable_tx_autodetection == 1)
//                lpmd->enable_tx_autodetection = 0;
//            else
//                lpmd->enable_tx_autodetection = 1;
//            dbg_printf("TX Autodetection is %s\n\r", lpmd->enable_tx_autodetection == 1 ? "enabled" : "disabled");
//            break;
        case 'b':
             mlconf->aux_log_enable = ~(mlconf->aux_log_enable);
             dbg_printf("Aux Logging is %s\n\r", mlconf->aux_log_enable  ? "enabled." : "disabled.");
             break;
        case 'c':
//        	 dbg_printf("\033[H\033[J"); //clears the screen
             dbg_printf("Run compliance test automation routines.\n\r");
             dbg_printf("Choose test option\n\r"
                        "1 --> train link @ 1.62G 1 lane\n\r"
                        "2 --> train link @ 1.62G 2 lanes\n\r"
                        "3 --> train link @ 1.62G 4 lanes\n\r"
                        "4 --> train link @  2.7G 1 lane\n\r"
                        "5 --> train link @  2.7G 2 lanes\n\r"
                        "6 --> train link @  2.7G 4 lanes\n\r"
                        "7 --> train link @  5.4G 1 lane\n\r"
                        "8 --> train link @  5.4G 2 lanes\n\r"
                        "9 --> train link @  5.4G 4 lanes\n\r"
#ifdef LLC_TEST_RUN
                        "A --> Video mode   640x480,  6bpc, llcramp - fail safe\n\r"
                        "B --> Video mode   640x480,  8bpc, llcramp \n\r"
                        "C --> Video mode   640x480, 10bpc, llcramp \n\r"
                        "D --> Video mode  1024x768,  6bpc, llcramp \n\r"
                        "E --> Video mode 1280x1024,  6bpc, llcramp \n\r"
                        "F --> Video mode 1600x1200,  6bpc, llcramp \n\r"
                        "G --> Video mode 1920x1200,  6bpc, llcramp \n\r"
#endif
                        "b --> set bits per color of video\n\r"
                        "m --> set video resolution\n\r"
                        "p --> set video pattern\n\r"
//                        "q --> set video pattern 2 (MST)\n\r"
//                        "r --> set video pattern 3 (MST)\n\r"
//                        "s --> set video pattern 4 (MST)\n\r"
//                        "M --> Enable MST Mode\n\r"
//                        "N --> Disable MST Mode\n\r"
                        "V --> Read DPCD Register Space\n\r"
                        "X --> Print Frame CRC Values\n\r"
            		 );

              command = xil_getc(0);
              switch(command){
              case '1':
                mlconf->link_rate = 6;
                mlconf->lane_count = 1;
                break;
              case '2':
                mlconf->link_rate = 6;
                mlconf->lane_count = 2;
                break;
              case '3':
                mlconf->link_rate = 6;
                mlconf->lane_count = 4;
                break;
              case '4':
                mlconf->link_rate = 0xA;
                mlconf->lane_count = 1;
                break;
              case '5':
                mlconf->link_rate = 0xA;
                mlconf->lane_count = 2;
                break;
              case '6':
                mlconf->link_rate = 0xA;
                mlconf->lane_count = 4;
                break;
              case '7':
                mlconf->link_rate = 0x14;
                mlconf->lane_count = 1;
                break;
              case '8':
                mlconf->link_rate = 0x14;
                mlconf->lane_count = 2;
                break;
              case '9':
                mlconf->link_rate = 0x14;
                mlconf->lane_count = 4;
                break;
#ifdef LLC_TEST_RUN
              case 'A':
                mlconf->pattern = 0x11;
                mlconf->bpc = 6;
                mlconf->video_mode_id = 0;
                break;
              case 'B':
                mlconf->pattern = 0x11;
                mlconf->bpc = 8;
                mlconf->video_mode_id = 0;
                break;
              case 'C':
                mlconf->pattern = 0x11;
                mlconf->bpc = 10;
                mlconf->video_mode_id = 0;
                break;
              case 'D':
                mlconf->pattern = 0x11;
                mlconf->bpc = 6;
                mlconf->video_mode_id = 2;
                break;
              case 'E':
                mlconf->pattern = 0x11;
                mlconf->bpc = 6;
                mlconf->video_mode_id = 5;
                break;
              case 'F':
                mlconf->pattern = 0x11;
                mlconf->bpc = 6;
                mlconf->video_mode_id = 7;
                break;
              case 'G':
                mlconf->pattern = 0x11;
                mlconf->bpc = 6;
                mlconf->video_mode_id = 9;
                break;
#endif
              case 'p':
                dbg_printf("Choose Video pattern\n\r"
                           "0 -->  Color Bars \n\r"
                           "1 -->  Vesa LLC pattern \n\r"
                           "2 -->  Vesa Pattern3 - bars \n\r"
                           "3 -->  Vesa Color Squares\n\r"
                           "4 -->  Flat Red  screen \n\r"
                           "5 -->  Flat Green screen \n\r"
                           "6 -->  Flat Blue screen \n\r"
                           "7 -->  Flat Yellow screen \n\r");
                term_key = xil_getc(0);
                switch (term_key){
                case'0':
                   mlconf->pattern = 0x10;
                   break;
                case'1':
                   mlconf->pattern = 0x11;
                   break;
                case'2':
                   mlconf->pattern = 0x12;
                   break;
                case'3':
                   mlconf->pattern = 0x13;
                   break;
                case'4':
                   mlconf->pattern = 0x14;
                   break;
                case'5':
                   mlconf->pattern = 0x15;
                   break;
                case'6':
                   mlconf->pattern = 0x16;
                   break;
                case'7':
                   mlconf->pattern = 0x17;
                   break;
                default:
                   mlconf->pattern = 0x0;
                   break;
                }

                if ( dptx_reg_read(XILINX_DISPLAYPORT_TX_USER_PIXEL_WIDTH) == 0x2 ) {
                   dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x308)),  ( 0x100 | mlconf->pattern ) );
                } else {
                   dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x308)),  mlconf->pattern );
                }

                dbg_printf("Video pattern option 0x0%01x selected..\n\r",mlconf->pattern & 0xf);
                break;

              case 'b':
                dbg_printf("Choose Video Bits per color option\n\r"
                           "0 -->  6 bpc \n\r"
                           "1 -->  8 bpc \n\r"
                           "2 --> 10 bpc \n\r"
                           "3 --> 12 bpc \n\r"
                           "4 --> 16 bpc \n\r" );
                term_key = xil_getc(0);
                switch (term_key){
                case'0':
                   mlconf->bpc = 6;
                   break;
                case'1':
                   mlconf->bpc = 8;
                   break;
                case'2':
                   mlconf->bpc = 10;
                   break;
                case'3':
                   mlconf->bpc = 12;
                   break;
                case'4':
                   mlconf->bpc = 16;
                   break;
                default:
                   break;
                }
                break;
#if RUN_DMT_MODES
              case 'm':
            	  xilcccModeHelp();
                dbg_printf("\n\rChoose DMT ID 2 hex char 0x");
                mlconf->video_mode_id = xil_gethex(2);
                dbg_printf("\n\r");
                break;
#else
              case 'm':
                dbg_printf("Choose Video Resolution option\n\r"
                           "0 -->   640x480 \n\r"
                           "1 -->   800x600 \n\r"
                           "2 -->  1024x768 \n\r"
                           "3 -->  1280x1024\n\r"
                           "4 -->  1600x1200\n\r"
                           "5 -->  1920x1200(RB)\n\r");
                term_key = xil_getc(0);
                switch (term_key){
                case'0':
                   mlconf->video_mode_id = 0;
                   break;
                case'1':
                   mlconf->video_mode_id = 1;
                   break;
                case'2':
                   mlconf->video_mode_id = 2;
                   break;
                case'3':
                   mlconf->video_mode_id = 5;
                   break;
                case'4':
                   mlconf->video_mode_id = 7;
                   break;
                case'5':
                   mlconf->video_mode_id = 9;
                   break;
                default:
                   mlconf->video_mode_id = 0;
                   break;
                }

                break;
#endif

                case 'V':

                  for(ii=0;ii<=524287;ii++)
                  {
                      status = xildpAUXRead(ii, 1, aux_data);
                      dbg_printf("[DPCD_CHECKS_READS] %x, %x, %x\r\n", ii, aux_data[0], status);
                  }

                  for(ii=0;ii<=524287;ii++)
                  {
                      status = xildpAUXWrite(ii, 1, 0x00);
                      dbg_printf("[DPCD_CHECKS_WRITES] %x, %x\r\n", ii, status);
                  }
                  break;


                case 'X':
                  dbg_printf("\n\r----------------- Frame CRC Values ------------------\r\n");
                  red_pixel0_crc   = dptx_reg_read((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x400)));
                  green_pixel0_crc = dptx_reg_read((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x404)));
                  blue_pixel0_crc  = dptx_reg_read((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x408)));
                  red_pixel1_crc   = dptx_reg_read((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x410)));
                  green_pixel1_crc = dptx_reg_read((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x414)));
                  blue_pixel1_crc  = dptx_reg_read((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x418)));
                  dbg_printf("-> [Stream 1][Pixel 0] Red CRC = 0x%x, Green CRC = 0x%x, Blue CRC = 0x%x\r\n", red_pixel0_crc, green_pixel0_crc, blue_pixel0_crc);
                  dbg_printf("-> [Stream 1][Pixel 1] Red CRC = 0x%x, Green CRC = 0x%x, Blue CRC = 0x%x\r\n", red_pixel1_crc, green_pixel1_crc, blue_pixel1_crc);
                  break;
              default:
            	  dbg_printf("\n\r Invalid Key Pressed. Press 'h' to display console \n\r");
              break;
              }

              if ( command == '1' || command == '2' || command == '3' ||
                   command == '4' || command == '5' || command == '6' ||
                   command == '7' || command == '8' || command == '9'  ) {
                   if ( mlconf->use_dpcd_caps == 1 ) {
                      mlconf->use_dpcd_caps = 0;
                      xildpSetLinkRate(mlconf->link_rate);
                      xildpSetLaneCount(mlconf->lane_count);
                      dplpmMaintainLink(1,1);
                      if(mlconf->link_rate == 0x14)
                      	xilcccChangeVideoMode(0,1,0x50,8);
                      else
                      	xilcccChangeVideoMode(0,1,0,6);
                      mlconf->use_dpcd_caps = 1;
                   } else {
                      xildpSetLinkRate(mlconf->link_rate);
                      xildpSetLaneCount(mlconf->lane_count);
                      dplpmMaintainLink(1,1);
                      if(mlconf->link_rate == 0x14)
						xilcccChangeVideoMode(0,1,0x50,8);
                      else
                      	xilcccChangeVideoMode(0,1,0,6);
                   }
              } else {
#ifdef LLC_TEST_RUN
				  if ( command == 'A' || command == 'B' || command == 'C' ||
					   command == 'D' || command == 'E' || command == 'F' ||
					   command == 'G' || command == 'm' || command == 'b' ||
					   command == 'c' || command == 'd' || command == 'y' ) {
					   xilcccChangeVideoMode(0,1,mlconf->video_mode_id,mlconf->bpc);
				  }
#else
				  if ( command == 'm' || command == 'b' || command == 'M' || command == 'N') {
					 xilcccChangeVideoMode(0,1,mlconf->video_mode_id,mlconf->bpc);
				  }
              }
#endif
         break;
         // Display MSA Values
         case 'd':
           xildpDisplayMSAValuesTX();
           xildpDisplayPatGen();

           break;

//         case 'e':
//           mlconf->edid_disable = ~(mlconf->edid_disable);
//           dbg_printf("Edid read %s\n\r", mlconf->edid_disable  ? "disabled" : "enabled");
//           break;
//
//         case 'f':
//            if (mlconf->sync_clock_mode == 1)
//                mlconf->sync_clock_mode = 0;
//            else
//                mlconf->sync_clock_mode = 1;
//
//            dbg_printf("MISC0[0]Synchronous Clock %s\n\r", mlconf->sync_clock_mode == 1 ? "enabled" : "disabled");
//            xilcccChangeVideoMode(0,1,mlconf->video_mode_id,mlconf->bpc);
//            break;

         // Normal, adaptive training
         case 'g':
            dbg_printf("Running adaptive training sequence\n\r");
            xildpResetPHYTX(XILINX_DISPLAYPORT_PHY_RESET_MASTER);
            dplpmMaintainLink(1,1);
            xilcccChangeVideoMode(0,1,0,6);
            break;

         // Adjust lane count
//         case 'l':
//            dplpmChangeLaneCount();
//            break;

//         // Main link enable toggle
//         case 'm':
//            lpmd->enable_main_stream = dptx_reg_read(XILINX_DISPLAYPORT_TX_ENABLE_MAIN_STREAM);
//            if (lpmd->enable_main_stream)
//                lpmd->enable_main_stream  = 0;
//            else
//                lpmd->enable_main_stream  = 1;
//            // Update the link enable status
//            dplpmMainLinkEnable(lpmd->enable_main_stream);
//            // Print out status message
//            dbg_printf("Main link %s\n\r", lpmd->enable_main_stream  ? "enabled" : "disabled");
//            break;
         // Status and training configuration
         case 's':
            dplpmReadDPCDStatus();
            break;
//          Adjust the link rate
//         case 'u':
//            dplpmChangeLinkRate();
//            break;

         // Change video mode
         case 'v':
            xilcccChangeVideoMode(1,0,0,mlconf->bpc);
            break;

         // Exit the application
         case 'x':
            done = 1;
            break;
         // sweep modes
//         case 'z':
//#if RUN_DMT_MODES
//            dbg_printf("Running video modes... setup link rate, lane count and bpc as required\n\r");
//            dbg_printf("Enter starting mode number - 2 hex characters 0x");
//            addr = xil_gethex(2);
//            xilRunmodes(addr);
//#else
//            dbg_printf("Running training on all lane/link combinations & video modes...\n\r");
//            xilRunmodes(0);
//#endif
//            break;

         case '1':
            // Set voltage swing levels
            dbg_printf("Please select a voltage swing setting\n\r"
                       " 1 -  400 mV\n\r"
                       " 2 -  600 mV\n\r"
                       " 3 -  800 mV\n\r"
                       " 4 -  1200 mV\n\r");
            term_key = xil_getc(0);
            switch(term_key){
            case '1':
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_0, XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_1, XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_2, XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_3, XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
                dbg_printf("Setting VSWING to 400mV\n\r");
                break;
            case '2':
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_0, XILINX_DISPLAYPORT_TX_PHY_VSWING_1);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_1, XILINX_DISPLAYPORT_TX_PHY_VSWING_1);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_2, XILINX_DISPLAYPORT_TX_PHY_VSWING_1);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_3, XILINX_DISPLAYPORT_TX_PHY_VSWING_1);
                dbg_printf("Setting VSWING to 600mV\n\r");
                break;
            case '3':
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_0, XILINX_DISPLAYPORT_TX_PHY_VSWING_2);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_1, XILINX_DISPLAYPORT_TX_PHY_VSWING_2);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_2, XILINX_DISPLAYPORT_TX_PHY_VSWING_2);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_3, XILINX_DISPLAYPORT_TX_PHY_VSWING_2);
                dbg_printf("Setting VSWING to 800mV\n\r");
                break;
            //case '4':
            default:
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_0, XILINX_DISPLAYPORT_TX_PHY_VSWING_3);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_1, XILINX_DISPLAYPORT_TX_PHY_VSWING_3);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_2, XILINX_DISPLAYPORT_TX_PHY_VSWING_3);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_3, XILINX_DISPLAYPORT_TX_PHY_VSWING_3);
                dbg_printf("Setting VSWING to 1200mV\n\r");
                break;
            }
            break;
         case '2':
            // Set preemphasis to preset value
            dbg_printf("Please select a preemphasis setting\n\r"
                       " 1 -   0 dB (1x)\n\r"
                       " 2 - 3.5 dB (1.5x)\n\r"
                       " 3 -   6 dB (2x)\n\r"
                       " 4 - 9.5 dB (3x)\n\r");
            term_key = xil_getc(0);
            switch(term_key){
            case '1':
                //7-series
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_0, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_1, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_2, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_3, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);

                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_0, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_0);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_1, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_0);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_2, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_0);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_3, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_0);
                dbg_printf("Setting PREEMP to 0dB\n\r");
                break;
            case '2':
                //7-series
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_0, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_1);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_1, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_1);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_2, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_1);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_3, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_1);

                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_0, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_1);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_1, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_1);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_2, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_1);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_3, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_1);
                dbg_printf("Setting PREEMP to 3.5dB\n\r");
                break;
            case '3':
                //7-series
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_0, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_2);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_1, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_2);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_2, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_2);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_3, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_2);

                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_0, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_2);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_1, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_2);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_2, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_2);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_3, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_2);
                dbg_printf("Setting PREEMP to 6dB\n\r");
                break;
            default: //case '4':
                //7-series
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_0, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_3);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_1, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_3);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_2, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_3);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_3, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_3);

                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_0, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_3);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_1, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_3);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_2, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_3);
                dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_3, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_3);
                dbg_printf("Setting PREEMP to 9.5dB\n\r");
                break;
            }
            break;

         // Adhere to DPCD capabilities in the sink device
//         case '3':
//            if (mlconf->use_dpcd_caps == 1) {
//                mlconf->use_dpcd_caps = 0;
//            } else {
//                mlconf->use_dpcd_caps = 1;
//            }
//            dbg_printf("Use of DPCD capabilities is %s\n\r", mlconf->use_dpcd_caps == 1 ? "enabled" : "disabled");
//            break;
         // Preserve link rate between RX and TX
//         case '4':
//            if (lpmd->preserve_linkrate == 1) {
//                lpmd->preserve_linkrate = 0;
//            } else {
//                lpmd->preserve_linkrate = 1;
//            }
//            dbg_printf("Preserve link rate is %s\n\r", lpmd->preserve_linkrate == 1 ? "enabled" : "disabled");
//            break;
         // Read EDID from sink device
         case ';':
            xildpDumpDPCD();
            status = dplpmReadEDID(&mlconf->ei, 0);
            break;

         // AXI read/write from host to all the slaves
         case 'A':
            dbg_printf("\n\rEnter 4 hex characters: Source Read address 0x");
            addr = xil_gethex(4);
            rdata = dptx_reg_read(XILINX_DISPLAYPORT_TX_BASE_ADDRESS+addr);
            dbg_printf("\n\rSource Read Addr %04x Read Data: %04x\n\r", (XILINX_DISPLAYPORT_TX_BASE_ADDRESS+addr), rdata);
            break;
         case 'C':
            dbg_printf("\n\rEnter 4 hex characters: Video Pattern Gen Read address 0x");
            addr = xil_gethex(4);
            rdata = dptx_reg_read(XILINX_DISPLAYPORT_VID_BASE_ADDRESS+addr);
            dbg_printf("\n\rVideo Pattern Gen Read Addr %04x Read Data: %04x\n\r", (XILINX_DISPLAYPORT_VID_BASE_ADDRESS+addr), rdata );
            break;
         case 'B':
            dbg_printf("\n\rEnter 4 hex characters: Source Write address 0x");
            addr = xil_gethex(4);
            dbg_printf("\n\rEnter 4 hex characters: Source Write data 0x");
            rdata = xil_gethex(4);
            dptx_reg_write((XILINX_DISPLAYPORT_TX_BASE_ADDRESS + addr), rdata);
            dbg_printf("\n\rSource Write Addr %04x Write Data: %04x\n\r", (XILINX_DISPLAYPORT_TX_BASE_ADDRESS+addr), rdata);
            break;
         case 'D':
            dbg_printf("\n\rEnter 4 hex characters: Video Pattern Gen Write address 0x");
            addr = xil_gethex(4);
            dbg_printf("\n\rEnter 4 hex characters: Video Pattern Gen Write Data 0x");
            rdata = xil_gethex(4);
            dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + addr), rdata);
            dbg_printf("\n\rVideo Pattern Gen Write Addr %04x Write Data: %04x\n\r", (XILINX_DISPLAYPORT_VID_BASE_ADDRESS+addr), rdata);
            break;
         // AUX to monitor on source transaction from Host
         case 'R':
            dbg_printf("\n\rEnter 4 hex characters: Aux Read Address 0x");
            addr = xil_gethex(4);
            xildpAUXRead(addr, 1, aux_data);
            dbg_printf("\n\rAux Read Addr %04x, Read Data: %02x\n\r", addr, aux_data[0]);
            break;
         case 'W':
            dbg_printf("\n\rEnter 4 hex characters: Aux Write Address 0x");
            addr = xil_gethex(4);
            dbg_printf("\n\rEnter 2 hex characters: Aux Write Data 0x");
            aux_data[0] = xil_gethex(2);
            xildpAUXWrite(addr, 1, aux_data);
            dbg_printf("\n\rAux Write Addr %04x Write Data: %02x\n\r", addr, aux_data[0] );
            break;
         case 'S':
            dptx_reg_write((XILINX_DISPLAYPORT_TX_AUDIO_CONTROL),  0x0000 );
            for(ii=0; ii<20; ii++){}
        //    sendAudioInfoFrame(xilInfoFrame);
            dptx_reg_write((XILINX_DISPLAYPORT_TX_AUDIO_CHANNELS),  0x0001 ); //Write Channel Count (actual count - 1)
            //Assuming, Audio Sample rate = 44.1 KHz, LInk Rate = 1.62G
            dptx_reg_write((XILINX_DISPLAYPORT_TX_AUDIO_MAUD),  225792 );
            dptx_reg_write((XILINX_DISPLAYPORT_TX_AUDIO_NAUD),  162 );
            dptx_reg_write((XILINX_DISPLAYPORT_TX_AUDIO_CONTROL),  0x0001 );
            dbg_printf("\n\r Audio Enabled");
            break;
         case 'T':
            dptx_reg_write((XILINX_DISPLAYPORT_TX_AUDIO_CONTROL),  0x0000 );
            dbg_printf("\n\r Audio Disabled");
            break;
         case '/':
        	 dbg_printf("\033[H\033[J");
        	 dbg_printf("\r\n Press 'h' to display console \r\n");
        	 break;
//         case 'E':
//            dptx_reg_write((XILINX_DISPLAYPORT_TX_SCRAMBLING_DISABLE),  0x1 );
//            dbg_printf("\n\r Scrambler Disabled");
//            break;
//         case 'F':
//            dptx_reg_write((XILINX_DISPLAYPORT_TX_SCRAMBLING_DISABLE),  0x0 );
//            dbg_printf("\n\r Scrambler Enabled");
//            break;
         default: //case 'h':
            xilcccDisplayHelp();
            break;
    }
    // Should only be '1' for exit condition
    return done;
}*/


/*
 *  Function:
 *
 *
 *  Parameters:
 *  None
 *
 *  Returns:
 *      None
 *
 *  Example:
 *
 *
 */
void xilcccDisplayHelp(void)
{
  dbg_printf("- - - - -  -  - - - - - - - - - - - - - - - - - -\n\r"
             "-                Select an Option               -\n\r"
             "- - - - - - - - - - - - - - - - - - - - - - - - -\n\r"
             "; - Read DPCD from the DisplayPort sink device\n\r"
             "b - Enable logging of AUX transactions\n\r"
		     "c - Run compliance test routines.\n\r"
             "d - Display MSA for TX\n\r"
             "g - Run standard adaptive training sequence\n\r"
             "h - Display this help menu\n\r"
             "s - Display DPCD status and training configuration\n\r"
             "1 - Adjust TX Voltage Swing\n\r"
             "2 - Adjust TX Preemphasis\n\r"
             "A - Read from SRC registers\n\r"
             "B - Write to SRC registers\n\r"
             "C - Read from Video Pattern Gen registers\n\r"
             "D - Write to Video Pattern Gen registers\n\r"
		     "S - Enable Audio \n\r"
		     "T - Disable Audio \n\r"
             "R - Read AUX Register\n\r"
             "W - Write AUX Register\n\r"
		     "x - Exit the application\n\r"
		     "/ - Clear screen\n\r"
             "- - - - - - - - - - - - - - - - - - - - - - - - - \n\r");
}

void xilcccModeHelp(void)
{
  dbg_printf("- - - - -  -  - - - - - - - - - - - - - - - - - - -  -  - - - - - - - - - - - - - - -\n\r"
             "-                            Select an Option                                       -\n\r"
             "- - - - - - - - - - - - - - - - - - - - - - - - - -  -  - - - - - - - - - - - - - - - \n\r"
		  "00 640x480_60_P     | 0b 1400x1050_60_P_RB | 1f 640x480_75_P   | 31 720x400_85_P \n\r"
		  "01 800x600_60_P     | 0c 1400x1050_60_P    | 20 800x600_75_P   | 32 640x480_85_P \n\r"
		  "02 848x480_60_P     | 0d 1440x900_60_P_RB  | 21 1024x768_75_P  | 33 800x600_85_P \n\r"
		  "03 1024x768_60_P    | 0e 1440x900_60_P     | 22 1152x864_75_P  | 34 1024x768_85_P \n\r"
		  "04 1280x768_60_P_RB | 0f 1600x1200_60_P    | 23 1280x768_75_P  | 35 1280x768_85_P \n\r"
		  "05 1280x768_60_P    | 10 1680x1050_60_P_RB | 24 1280x800_75_P  | 36 1280x800_85_P \n\r"
		  "06 1280x800_60_P_RB | 11 1680x1050_60_P    | 25 1280x1024_75_P | 37 1280x960_85_P \n\r"
		  "07 1280x800_60_P    | 19 800x600_56_P      | 26 1400x1050_75_P | 4f 1366x768_60_P \n\r"
		  "08 1280x960_60_P    | 1c 1024x768_70_P     | 27 1440x900_75_P  | 50 1080p@60     \n\r"
		  "09 1280x1024_60_P   | 1d 640x480_72_P      | 2f 640x350_85_P   |              \n\r"
		  "0a 1360x768_60_P    | 1e 800x600_72_P      | 30 640x400_85_P   |               \n\r"
          "- - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n\r");
}

UINT32 xilcccChangeVideoMode(UINT8 inc, UINT8 set_mode, UINT8 mode, UINT8 bpc) {

	XILDPMainLinkConfig *mlconf = dplpmGetMainLinkConfig();
	static XilVideoPatternRegisters xilVPR;
	static XILDPMainStreamAttributes xilMSA;
	UINT8 link_rate_int = mlconf->link_rate;
	UINT32 bpp_i;

	// disable main stream - to force sending of IDLE patterns
	dplpmMainLinkEnable(0);

	dptx_reg_write(XILINX_DISPLAYPORT_TX_SOFT_VIDEO_RESET, 0xF);   // Assert the reset
	dptx_reg_write(XILINX_DISPLAYPORT_TX_SOFT_VIDEO_RESET, 0x0);   // Deassert the reset

	xildpClearMSAValues();

	// default and basic computations
	xilVPR.vid_clk_sel            = (link_rate_int == 0xa);

	if(link_rate_int == 0x14)
	xilVPR.vid_clk_D              = 270;
	else if(link_rate_int == 0xa)
	xilVPR.vid_clk_D              = 135;
	else
	xilVPR.vid_clk_D              = 81;

	xilMSA.user_pixel_width       = 1; // default single pixel mode
	xilVPR.de_polarity            = 0x0;
	xilVPR.framelock0             = 0;
	xilVPR.framelock1             = 0;
	xilVPR.hdcolorbar_mode        = 0;

	xilVPR.vsync_polarity         = dmt_modes[mode].vpol;
	xilVPR.hsync_polarity         = dmt_modes[mode].hpol;
	xilVPR.vsync_pulse_width      = dmt_modes[mode].vsw;
	xilVPR.vertical_back_porch    = dmt_modes[mode].vbp;
	xilVPR.vertical_front_porch   = dmt_modes[mode].vfp;
	xilVPR.vertical_resolutions   = dmt_modes[mode].vres;
	xilVPR.hsync_pulse_width      = dmt_modes[mode].hsw;
	xilVPR.horizontal_back_porch  = dmt_modes[mode].hbp;
	xilVPR.horizontal_front_porch = dmt_modes[mode].hfp;
	xilVPR.horizontal_resolutions = dmt_modes[mode].hres;
	xilVPR.vid_clk_M              = dmt_modes[mode].vclk;

	//----------------------------------------------
	// Configure main stream attributes(MSA)
	//----------------------------------------------
	xilMSA.h_clk_total       = (xilVPR.hsync_pulse_width + xilVPR.horizontal_back_porch +
							   xilVPR.horizontal_front_porch + xilVPR.horizontal_resolutions);
	xilMSA.v_clk_total       = (xilVPR.vsync_pulse_width + xilVPR.vertical_back_porch +
							   xilVPR.vertical_front_porch + xilVPR.vertical_resolutions);
	xilMSA.v_sync_polarity   = xilVPR.vsync_polarity;
	xilMSA.h_sync_polarity   = xilVPR.hsync_polarity;
	xilMSA.h_sync_width      = xilVPR.hsync_pulse_width;
	xilMSA.v_sync_width      = xilVPR.vsync_pulse_width;
	xilMSA.h_resolution      = xilVPR.horizontal_resolutions;
	xilMSA.v_resolution      = xilVPR.vertical_resolutions;
	xilMSA.h_start           = (xilVPR.hsync_pulse_width + xilVPR.horizontal_back_porch);
	xilMSA.v_start           = xilVPR.vsync_pulse_width + xilVPR.vertical_back_porch;

	xilMSA.m_vid             = xilVPR.vid_clk_M;
	xilMSA.n_vid             = (xilVPR.vid_clk_D) * 2 * 1000;

	//--------------------------------------------------------------
	// compute misc0 and date_per_lane based on bpc.
	//--------------------------------------------------------------
	bpp_i = (mlconf->comp_fmt==1) ? bpc * 2:bpc * 3;

	switch (bpc) {
	  case 6:
		 xilMSA.misc0 = 0x00;
		 break;
	  case 8:
		 xilMSA.misc0 = 0x20;
		 break;
	  case 10:
		 xilMSA.misc0 = 0x40;
		 break;
	  case 12:
		 xilMSA.misc0 = 0x60;
		 break;
	  case 16:
		 xilMSA.misc0 = 0x80;
		 break;
	  default:
		 xilMSA.misc0 = 0x00;
		 break;
	}

	xilMSA.misc0 = xilMSA.misc0 | (mlconf->comp_fmt<<1) | (mlconf->dyn_range<<3) | (mlconf->ycbcr_color<<4);
	xilMSA.misc0 = (xilMSA.misc0 | (mlconf->sync_clock_mode & 0x1));
	//xilMSA.misc0 = 0x21;
	//xilMSA.misc1 = 0;
	xilMSA.misc1 = 0x00;

	xilMSA.data_per_lane = (xilVPR.horizontal_resolutions *  bpc * 3 /16) - mlconf->lane_count;

	//--------------------------------------------------------------
	// compute TU parameters
	//--------------------------------------------------------------
	xilMSA.transfer_unit_size = xilSetTU(xilVPR.vid_clk_M, bpp_i, mlconf->link_rate, mlconf->lane_count, mlconf->mst_enable);

	//--------------------------------------------------------------
	// re-compute horizontal values based on user pixel width.
	// this will apply to clock generation and pattern generator setup
	//--------------------------------------------------------------
	/*xilVPR.horizontal_resolutions  =   xilVPR.horizontal_resolutions  / xilMSA.user_pixel_width;
	xilVPR.horizontal_back_porch   =   xilVPR.horizontal_back_porch   / xilMSA.user_pixel_width;
	xilVPR.horizontal_front_porch  =   xilVPR.horizontal_front_porch  / xilMSA.user_pixel_width;
	xilVPR.hsync_pulse_width       =   xilVPR.hsync_pulse_width       / xilMSA.user_pixel_width;
	xilVPR.vid_clk_M               =   xilVPR.vid_clk_M               / xilMSA.user_pixel_width;*/

	//--------------------------------------------------------------
	// compute video clock parameters
	//--------------------------------------------------------------
	/*UINT32 ref_clk;

	if(link_rate_int == 0x14)
	ref_clk = 270000;
	else if(link_rate_int == 0xa)
	ref_clk = 135000;
	else
	  ref_clk = 81000;

	UINT32 vid_clk   = xilVPR.vid_clk_M;
	xilComputeVidMVid ( vid_clk, ref_clk, &xilVPR );*/

	//--------------------------------------------------------------
	// configure the pattern generator
	//--------------------------------------------------------------
	/*xilVPR.tc_hsblnk = xilVPR.horizontal_resolutions - 1;
	xilVPR.tc_hssync = xilVPR.horizontal_resolutions + xilVPR.horizontal_front_porch - 1 ;
	xilVPR.tc_hesync = xilVPR.horizontal_resolutions + xilVPR.horizontal_front_porch +
					 xilVPR.hsync_pulse_width - 1 ;
	xilVPR.tc_heblnk = xilVPR.horizontal_resolutions + xilVPR.horizontal_front_porch +
					 xilVPR.hsync_pulse_width + xilVPR.horizontal_back_porch - 1;

	xilVPR.tc_vsblnk = xilVPR.vertical_resolutions - 1;
	xilVPR.tc_vssync = xilVPR.vertical_resolutions + xilVPR.vertical_front_porch - 1 ;
	xilVPR.tc_vesync = xilVPR.vertical_resolutions + xilVPR.vertical_front_porch +
					 xilVPR.vsync_pulse_width - 1;
	xilVPR.tc_veblnk = xilVPR.vertical_resolutions + xilVPR.vertical_front_porch +
					 xilVPR.vsync_pulse_width + xilVPR.vertical_back_porch - 1 ;*/

	//--------------------------------------------------------------
	// set up msa and patgen
	//--------------------------------------------------------------
	xildpSetMSAValues(&xilMSA);

	//--------------------------------------------------------------
	// enable main stream - before starting new video..
	//--------------------------------------------------------------
	dplpmMainLinkEnable(1);

	return 1;
}


void read_allocation_table(void)
{
	UINT8 m=0;
    UINT8 aux_data[64];
    UINT32 status;

	// Read allocation table
    for(m=0; m<63; m=m+16)
    {
        status = xildpAUXRead(XILINX_DISPLAYPORT_DPCD_VC_PAYLOAD_ID_SLOT+m, 16, &aux_data[m]);
    }

    if ( !status ) {
        dbg_printf("\n\r======================================  VCPayload Table =======================================\n\r");
        for(m = 0; m < 63; m+=16){
            dbg_printf("VCPAYLOAD_TABLE[%03d to %03d] %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, "
            		                                 "%02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n\r",
            		m+1, m+17,
            		aux_data[m],   aux_data[m+1], aux_data[m+2], aux_data[m+3],
            		aux_data[m+4], aux_data[m+5], aux_data[m+6], aux_data[m+7],
            		aux_data[m+8], aux_data[m+9], aux_data[m+10],aux_data[m+11],
            		aux_data[m+12],aux_data[m+13],aux_data[m+14],aux_data[m+15]);
        }
        dbg_printf("===================================================================================================\n\r\n\r");
    } else {
    	dbg_printf("Reading VCPayload Table Failed...\n\r");
    }

}

/* function to compute TU packing
*  based on link rate and video rate determines the TU programming
*/
UINT32 xilSetTU (UINT32 vid_clk, UINT32 bpp, UINT32 link_rate, UINT32 lanes, UINT8 mst_enable){
    UINT32 tu = 64;
    UINT32 video_bw;
    UINT32 link_freq;
    UINT32 avg_bytes_per_tu;
    UINT32 frac;

    video_bw = (vid_clk * bpp)/8;

    if(link_rate == 0x14)
    	link_freq = 540;
    else if (link_rate == 0xA)
    	link_freq = 270;
    else
    	link_freq = 162;

  	avg_bytes_per_tu = (video_bw*tu)/(lanes*link_freq);

    frac = avg_bytes_per_tu%1000;
//    dbg_printf ("clk %d, bpp %d, link_freq = %d, lanes = %d, avg_bytes_per_tu = %d, frac = %d, min_bytes_per_tu = %d\n\r",
//                 vid_clk, bpp, link_freq, lanes, avg_bytes_per_tu, frac,(avg_bytes_per_tu/1000));

    dptx_reg_write(XILINX_DISPLAYPORT_TX_MIN_BYTES_PER_TU, (avg_bytes_per_tu/1000));
    dptx_reg_write(XILINX_DISPLAYPORT_TX_FRAC_BYTES_PER_TU, frac);
    if ( (avg_bytes_per_tu/1000) > tu ) {
       dbg_printf ("\r\nWarning! Link is over subscribed! Link BW = %d KBps; Video BW = %d KBps",(link_freq*lanes*1000),video_bw);
    } else {
//       dbg_printf ("Link BW %d KBps, Video BW %d KBps\n\r",(link_freq*lanes*1000),video_bw);
    }

    UINT32 init_wait = ( tu - (avg_bytes_per_tu/1000) );
    if ( (avg_bytes_per_tu/1000) > tu ) {
       init_wait = 0;
    } else if ( init_wait > 10 ) {
       init_wait = init_wait - 10;
    } else {
    	init_wait = 0;
    }

    dptx_reg_write(XILINX_DISPLAYPORT_TX_INIT_WAIT, init_wait);

    // TU packing set to max since in MST throttling happens as per VC Payload management
    if(mst_enable)
    {
        dptx_reg_write(XILINX_DISPLAYPORT_TX_MIN_BYTES_PER_TU, 32);
        dptx_reg_write(XILINX_DISPLAYPORT_TX_FRAC_BYTES_PER_TU, 0);
        dptx_reg_write(XILINX_DISPLAYPORT_TX_INIT_WAIT, 1);
    }
//    dbg_printf("Init wait %d\n\r",init_wait);

    return tu;
}



/*void xilSetPatternGeneratorRegisters(XilVideoPatternRegisters *ptrVideoPat)
{
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x04)),  ptrVideoPat->vsync_polarity);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x08)),  ptrVideoPat->hsync_polarity);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x0C)),  ptrVideoPat->de_polarity);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x10)),  ptrVideoPat->vsync_pulse_width);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x14)),  ptrVideoPat->vertical_back_porch);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x18)),  ptrVideoPat->vertical_front_porch);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x1C)),  ptrVideoPat->vertical_resolutions);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x20)),  ptrVideoPat->hsync_pulse_width);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x24)),  ptrVideoPat->horizontal_back_porch);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x28)),  ptrVideoPat->horizontal_front_porch);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x2C)),  ptrVideoPat->horizontal_resolutions);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x34)),  ptrVideoPat->framelock0);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x3C)),  ptrVideoPat->framelock1);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x40)),  ptrVideoPat->hdcolorbar_mode);

  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x44)),  ptrVideoPat->tc_hsblnk);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x48)),  ptrVideoPat->tc_hssync);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x4C)),  ptrVideoPat->tc_hesync);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x50)),  ptrVideoPat->tc_heblnk);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x54)),  ptrVideoPat->tc_vsblnk);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x58)),  ptrVideoPat->tc_vssync);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x5C)),  ptrVideoPat->tc_vesync);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x60)),  ptrVideoPat->tc_veblnk);

  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x100)), ptrVideoPat->vid_clk_sel);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x104)), ptrVideoPat->vid_clk_M);
  dptx_reg_write((XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x108)), ptrVideoPat->vid_clk_D);

}*/


void xilComputeVidMVid ( UINT32 vid_freq, UINT32 ref_freq, XilVideoPatternRegisters *ptrVideoPat) {
        UINT32 m, d, div, freq, diff, fvco;
        UINT32 minerr = 10000;
        UINT32 mval = 0, dval = 0, divval = 0, freqval = 0, diffval = 0, fvcoval = 0;

        for ( m = 20; m <= 64; m++ ) {
           for ( d = 1; d <= 80; d++ ) {
              fvco = ref_freq * m / d;
              if ( fvco >= 600000 && fvco <= 900000 ){
                 for (div = 1; div <= 128; div ++ ){
                    freq = fvco/div;

                    if ( freq >= vid_freq )
                       diff = freq - vid_freq;
                    else
                       diff = vid_freq - freq;

                    if ( diff == 0 ){
                       //dbg_printf ("Found M %d D %d Div %d Ref_freq = %d, Vid_freq = %d, Computed = %d, Err %d, fvco = %d\n\r",m,d,div,ref_freq,vid_freq,freq,diff,fvco);
                       mval = m; dval = d; divval = div; freqval = freq; diffval = minerr; fvcoval = fvco;
                       m = 257;d = 257;div = 257;minerr = 0;
                    } else if ( diff < minerr ) {
                       minerr = diff;
                       //dbg_printf ("Found with Error M %d D %d Div %d Ref_freq = %d, Vid_freq = %d, Computed = %d, Err %d, fvco = %d\n\r",m,d,div,ref_freq,vid_freq,freq,diff,fvco);
                       mval = m; dval = d; divval = div; freqval = freq; diffval = minerr; fvcoval = fvco;
                       if ( minerr < 100 ) {
                           m = 257;d = 257;div = 257;
                       }
                    }
                 }
              }
           }
        }
        ptrVideoPat->vid_clk_M = mval;
        ptrVideoPat->vid_clk_D = (divval & 0xff) | ((dval & 0xff)<<8);
//        dbg_printf ("Found M %d D %d Div %d  Ref_freq = %d, Vid_freq = %d, Computed = %d, Err %d, fvco = %d\n\r",mval,dval,divval, ref_freq,vid_freq,freqval,minerr,fvcoval);
        return;
}


/* function to wait specified number of vsync from patgen */
/*void wait_tx_vsyncs ( UINT32 value ){
     volatile UINT32 vcount, veblank, loop;
     volatile UINT32 start, end, diff = 0;
     loop = value;
     veblank = dptx_reg_read(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x60));
     vcount  = dptx_reg_read(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x200));
     start   = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT + 0x14);

     while ( loop > 0 ) {
        end  = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT + 0x14);
        if ( start > end ) diff = start - end;
        else               diff = end - start;
        //dbg_printf ( "start %d end %d %d, %d, %d, delay %d\n\r",start, end, value-loop,vcount, veblank, diff );
        if ( vcount >= veblank ){
           //dbg_printf ( "Seen vsync %d, %d, %d, delay %d\n\r",value-loop,vcount, veblank, diff );
           loop--;
           diff = 0;
        } else if ( diff > 100000000 ) {
           loop = 0;
           dbg_printf ( "start %d end %d %d, %d, %d, delay %d\n\r",start, end, value-loop,vcount, veblank, diff );
        }
        vcount  = dptx_reg_read(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x200));
     }
}*/

