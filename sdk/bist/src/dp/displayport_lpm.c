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


#include "stdio.h"
#include "xlib_string.h"
#include "displayport_tx_drv.h"
#include "xil_displayport.h"
#include "xil_ccc_app.h"
#include "displayport_lpm.h"
#include "mode_table.h"
#include "microblaze_sleep.h"

/*
 *  Function: 
 *      Main thread of execution for the link policy maker.
 *
 *  Parameters:
 *		
 *
 *  Returns:
 *      (UINT32) Status code; zero indicates success, non-zero indicates an error condition
 *
 *
 */
void dplpmInitLinkPolicyMaker(UINT32 link_rate, UINT32 lane_count)
{
    XILDPLinkPolicyMakerData *lpmdata = dplpmGetLinkPolicyMakerData();

    lpmdata->misc0 = 0x00;
    lpmdata->pm = 1;
    lpmdata->enable_main_stream = 0; 
    lpmdata->enable_secondary_stream = 0;
    lpmdata->enable_tx_autodetection = 1;
    lpmdata->enable_rx_autodetection = 0;
    lpmdata->bypass_msa_filter = 0;
    lpmdata->reset_type = 1;
    lpmdata->reset_phy = 0;
    lpmdata->reset_phy_string = "RX Only";
    lpmdata->mlconf = dplpmGetMainLinkConfig();
    lpmdata->preserve_linkrate = 0;

    // Base values for link configuration
    lpmdata->mlconf->lane_count = lane_count;
    lpmdata->mlconf->link_rate = link_rate;
    lpmdata->mlconf->training_settings = 0;
    lpmdata->mlconf->use_preferred_mode = 1;
    lpmdata->mlconf->use_dpcd_caps = 0;
    lpmdata->mlconf->failsafe = 0;
    lpmdata->mlconf->bpc = 8;
    lpmdata->mlconf->pattern = 0x11;
    lpmdata->mlconf->pattern2 = 0x10;
    lpmdata->mlconf->pattern3 = 0x12;
    lpmdata->mlconf->pattern4 = 0x14;
    lpmdata->mlconf->video_mode_id = 0;
    lpmdata->mlconf->sync_clock_mode = 0;
    lpmdata->mlconf->aux_log_enable = 0;
    lpmdata->mlconf->mst_enable = 0;
    lpmdata->mlconf->no_of_mst_streams = 1;

    // Enhanced framing mode , spread spectrum and main link channel coding
    lpmdata->mlconf->efm = 1;
    lpmdata->mlconf->ssc = 0;
    lpmdata->mlconf->mlc = 1;

    // Display starting information
    dbg_printf("\r\nLink Policy Maker Active:");
    dbg_printf("\r\n  > TX Auto-detection is %s",
    		lpmdata->enable_tx_autodetection == 1 ? "enabled" : "disabled");
    dbg_printf("\r\n  > MSA Filter Bypass is %s",
    		lpmdata->bypass_msa_filter == 1 ? "enabled" : "disabled");
    dbg_printf("\r\nDefault Configuration:");
    dbg_printf("\r\n  > Link Rate  : %s",
    		lpmdata->mlconf->link_rate == 0x14 ? "High (5.4Gbps)" :
    		(lpmdata->mlconf->link_rate == 0x0A ? "Med (2.7Gbps)" : "Low (1.62Gbps)"));
    dbg_printf("\r\n  > Lane Count : %d", lpmdata->mlconf->lane_count & 0x07);
}

/*
 *  Function: 
 *      HPD detection loop
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
UINT32 dplpmLinkPolicyMakerRunLoop(void)
{
    XILDPLinkPolicyMakerData *lpmdata = dplpmGetLinkPolicyMakerData();
    UINT32 status = 0;


    // Auto-detection loop
    if (lpmdata->enable_tx_autodetection)
    {
        while(dplpmCheckHPDStatus() != HPD_STATE_CONNECTED);
    }

    return status;
}

/*
 *  Function: 
 *      
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
UINT32 dplpmTrainLink(UINT32 training_settings, UINT32 preserve_linkrate, UINT32 force)
{
    UINT32 status = 0;
    dplpmMainLinkEnable(0);
    if (force)
        status = xildpRunTrainingLoop(training_settings, 0, 1);
    else
        status = xildpRunTrainingLoop(training_settings, 1, preserve_linkrate);
    //dbg3_printf("Status : 0x%08x\n\r", status);
    return status;
}

/*
 *  Function: 
 *      
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
UINT32 dplpmReadDPCDStatus(void)
{
    UINT32 status = 0, xx = 0;
    UINT8 aux_data[16];
    UINT8 aux_data1[16];

    status = xildpAUXRead(XILINX_DISPLAYPORT_DPCD_LINK_BANDWIDTH_SET, 8, aux_data);

    if(aux_data[0]==0x14){
    	dbg_printf("\r\n Link : 5.4x");
    }
    else if(aux_data[0]==0x0A){
    	dbg_printf("\r\n Link : 2.7x");
    }
    else{
    	dbg_printf("\r\n Link : 1.62x");
    }

    if((aux_data[1]&0x7)==0x04){
    	dbg_printf("4");
    }
    else if((aux_data[1]&0x7)==0x02){
    	dbg_printf("2");
    }
    else{
    	dbg_printf("1");
    }
    status = xildpAUXRead(XILINX_DISPLAYPORT_DPCD_STATUS_LANE_0_1, 6, aux_data1);
    if (status == XILINX_DISPLAYPORT_OPERATION_SUCCESS)
    {
    	if((aux_data[1]&0x7)==0x04)
    	{
    		if(((aux_data1[0])==0x77) & (aux_data1[1]==0x77)){
    			dbg_printf(" ->  DP Sink Trained\r\n");
    			dbg_printf("\n\r Adjust Request Lane 0: VS->Level %x, PRE-EMP->Level %x"
    					"\n\r Adjust Request Lane 1: VS->Level %x, PRE-EMP->Level %x "
    					"\n\r Adjust Request Lane 2: VS->Level %x, PRE-EMP->Level %x "
    					"\n\r Adjust Request Lane 3: VS->Level %x, PRE-EMP->Level %x \n\r",
    					(aux_data1[4]&0x03),(aux_data1[4]&0x0C)>>2,(aux_data1[4]&0x30)>>4,(aux_data1[4]&0xC0)>>6,
    					(aux_data1[5]&0x03),(aux_data1[5]&0x0C)>>2,(aux_data1[5]&0x30)>>4,(aux_data1[5]&0xC0)>>6
    					);
    		}
    		else{
				dbg_printf("\n\r \n\r"
						   " Status         |  Lane 0  |  Lane 1  |  Lane 2  |  Lane 3  \n\r"
						   "-------------------------------------------------------------------\n\r"
						   " CR Done        |     %s    |     %s    |     %s    |     %s \n\r"
						   "-------------------------------------------------------------------\n\r"
						   " Channel EQ Done|     %s    |     %s    |     %s    |     %s \n\r"
						   "-------------------------------------------------------------------\n\r"
						   " Symbol Locked  |     %s    |     %s    |     %s    |     %s \n\r",
						   ((aux_data1[0]&0x01)==0x01 ? "Y" : "N"),
						   ((aux_data1[0]&0x10)==0x10 ? "Y" : "N"),
						   ((aux_data1[1]&0x01)==0x01 ? "Y" : "N"),
						   ((aux_data1[1]&0x10)==0x10 ? "Y" : "N"),
						   ((aux_data1[0]&0x02)==0x02 ? "Y" : "N"),
						   ((aux_data1[0]&0x20)==0x20 ? "Y" : "N"),
						   ((aux_data1[1]&0x02)==0x02 ? "Y" : "N"),
						   ((aux_data1[1]&0x20)==0x20 ? "Y" : "N"),
						   ((aux_data1[0]&0x04)==0x04 ? "Y" : "N"),
						   ((aux_data1[0]&0x40)==0x40 ? "Y" : "N"),
						   ((aux_data1[1]&0x04)==0x04 ? "Y" : "N"),
						   ((aux_data1[1]&0x40)==0x40 ? "Y" : "N"));
    		}
    	} else if((aux_data[1]&0x7)==0x02)
    	{
    		if(aux_data1[0]==0x77){
    			dbg_printf(" ->  DP Sink Trained\r\n");
    			dbg_printf("\n\r Adjust Request Lane 0: VS->Level %x, PRE-EMP->Level %x "
    					"\n\r Adjust Request Lane 1: VS->Level %x, PRE-EMP->Level %x \n\r",
    					(aux_data1[4]&0x03),(aux_data1[4]&0x0C)>>2,(aux_data1[4]&0x30)>>4,(aux_data1[4]&0xC0)>>6
    					);
    		}
    		else{
				dbg_printf(" \n\r"
						   " Status         |  Lane 0  |  Lane 1   \n\r"
						   "-----------------------------------------\n\r"
						   " CR Done        |     %s    |     %s     \n\r"
						   "-----------------------------------------\n\r"
						   " Channel EQ Done|     %s    |     %s     \n\r"
						   "-----------------------------------------\n\r"
						   " Symbol Locked  |     %s    |     %s     \n\r",
						   ((aux_data1[0]&0x01)==0x01 ? "Y" : "N"),
						   ((aux_data1[0]&0x10)==0x10 ? "Y" : "N"),
						   ((aux_data1[0]&0x02)==0x02 ? "Y" : "N"),
						   ((aux_data1[0]&0x20)==0x20 ? "Y" : "N"),
						   ((aux_data1[0]&0x04)==0x04 ? "Y" : "N"),
						   ((aux_data1[0]&0x40)==0x40 ? "Y" : "N"));
    		}
    	} else
    	{
    		if(aux_data1[0]==0x07){
    			dbg_printf(" ->  DP Sink Trained\r\n");
    			dbg_printf("\n\r Adjust Request Lane 0: VS->Level %x, PRE-EMP->Level %x \n\r",
    					(aux_data1[4]&0x03),(aux_data1[4]&0x0C)>>2
    					);
    		}
    		else{
					dbg_printf(" \n\r"
							   " Status         |  Lane 0   \n\r"
							   "----------------------------\n\r"
							   " CR Done        |     %s    \n\r"
							   "----------------------------\n\r"
							   " Channel EQ Done|     %s    \n\r"
							   "----------------------------\n\r"
							   " Symbol Locked  |     %s     \n\r",
							   ((aux_data1[0]&0x01)==0x01 ? "Y" : "N"),
							   ((aux_data1[0]&0x02)==0x02 ? "Y" : "N"),
							   ((aux_data1[0]&0x04)==0x04 ? "Y" : "N"));
    		}
    	}

    	dbg_printf("\n\r - - - - - - Additional DisplayPort Register Values - - - - - -\n\r");
        dbg_printf("\n\r"
        		"Address | Register Name                Value \n\r"
        		"(0x0204) Lane Align Status Updated    : 0x%x \n\r"
        		"(0x0205) Sink Status                  : 0x%x \n\r"
        		"(0x0206) Adjust Request Lane 01       : 0x%x \n\r"
        		"(0x0207) Adjust Request Lane 23       : 0x%x \n\r",
        		aux_data1[2],aux_data1[3],aux_data1[4],aux_data1[5]
        		);
    }

    if (status == XILINX_DISPLAYPORT_OPERATION_SUCCESS)
    {
        for (xx = 0; xx < 8; xx++)
            dbg_printf("(0x%04x) %s : 0x%02x\n\r", XILINX_DISPLAYPORT_DPCD_LINK_BANDWIDTH_SET + xx, xildpGetDPCDRegName(XILINX_DISPLAYPORT_DPCD_LINK_BANDWIDTH_SET + xx), aux_data[xx]);
    }
    dbg_printf("- - - - - - - - - - - - - - - - - - - - - - - -\n\r");
    return status;
}

/*
 *  Function: 
 *      
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
UINT32 dplpmReadStatus(void)
{
    UINT32 status = 0;
    UINT8 aux_data[16];
    UINT8 aux_data1[16];
//    dbg_printf("\033[u\033[J");//->recovers cursor position
//    dbg_printf("\033[s");//->saves cursor position
    status = xildpAUXRead(XILINX_DISPLAYPORT_DPCD_LINK_BANDWIDTH_SET, 8, aux_data);

    status = xildpAUXRead(XILINX_DISPLAYPORT_DPCD_STATUS_LANE_0_1, 6, aux_data1);
    if (status == XILINX_DISPLAYPORT_OPERATION_SUCCESS)
    {
    	if((aux_data[1]&0x7)==0x04)
    	{
    		if(((aux_data1[0])==0x77) & (aux_data1[1]==0x77)){
    			dbg_printf("\r\n> DisplayPort Sink Trained");
    			dbg_printf("\r\n  Config: ");
    		    if(aux_data[0]==0x14){
    		    	dbg_printf("Link BW & Count -> 5.4x");
    		    }
    		    else if(aux_data[0]==0x0A){
    		    	dbg_printf("Link BW & Count -> 2.7x");
    		    }
    		    else{
    		    	dbg_printf("Link BW & Count -> 1.62x");
    		    }

    		    if((aux_data[1]&0x7)==0x04){
    		    	dbg_printf("4\r\n");
    		    }
    		    else if((aux_data[1]&0x7)==0x02){
    		    	dbg_printf("2\r\n");
    		    }
    		    else{
    		    	dbg_printf("1\r\n");
    		    }
    			///////////////////////////////////////////////////////////////////////////////////////////
    		}
    		else{
#if PRINT_STATUS
    			dbg_printf("\n\r");
				dbg_printf(" \r\n Lane 0,1,2,3 Status: CR Done?%s,%s,%s,%s; Channel EQ Done?%s,%s,%s,%s; Symbol Locked?%s,%s,%s,%s\n\r",
						   ((aux_data1[0]&0x01)==0x01 ? "Y" : "N"),
						   ((aux_data1[0]&0x10)==0x10 ? "Y" : "N"),
						   ((aux_data1[1]&0x01)==0x01 ? "Y" : "N"),
						   ((aux_data1[1]&0x10)==0x10 ? "Y" : "N"),
						   ((aux_data1[0]&0x02)==0x02 ? "Y" : "N"),
						   ((aux_data1[0]&0x20)==0x20 ? "Y" : "N"),
						   ((aux_data1[1]&0x02)==0x02 ? "Y" : "N"),
						   ((aux_data1[1]&0x20)==0x20 ? "Y" : "N"),
						   ((aux_data1[0]&0x04)==0x04 ? "Y" : "N"),
						   ((aux_data1[0]&0x40)==0x40 ? "Y" : "N"),
						   ((aux_data1[1]&0x04)==0x04 ? "Y" : "N"),
						   ((aux_data1[1]&0x40)==0x40 ? "Y" : "N"));
#endif
    		}
    	} else if((aux_data[1]&0x7)==0x02)
    	{
    		if(aux_data1[0]==0x77){
    			dbg_printf("\r\nDisplayPort Main Link Active");
				dbg_printf("\r\n  > Config: ");
				if(aux_data[0]==0x14){
					dbg_printf("Link BW & Count -> 5.4 Gbps;");
				}
				else if(aux_data[0]==0x0A){
					dbg_printf("Link BW & Count -> 2.7 Gbps;");
				}
				else{
					dbg_printf("Link BW & Count -> 1.62 Gbps;");
				}

				if((aux_data[1]&0x7)==0x04){
					dbg_printf(" 4-lanes");
				}
				else if((aux_data[1]&0x7)==0x02){
					dbg_printf(" 2-lanes");
				}
				else{
					dbg_printf(" 1-lane");
				}
    		}
    		else{
#if PRINT_STATUS
				dbg_printf(" \n\n Lane 0,1 Status: CR Done?%s,%s; Channel EQ Done?%s,%s; Symbol Locked?%s,%s\n\r",
						   ((aux_data1[0]&0x01)==0x01 ? "Y" : "N"),
						   ((aux_data1[0]&0x10)==0x10 ? "Y" : "N"),
						   ((aux_data1[0]&0x02)==0x02 ? "Y" : "N"),
						   ((aux_data1[0]&0x20)==0x20 ? "Y" : "N"),
						   ((aux_data1[0]&0x04)==0x04 ? "Y" : "N"),
						   ((aux_data1[0]&0x40)==0x40 ? "Y" : "N"));
#endif
    		}
    	} else
    	{
    		if(aux_data1[0]==0x07){
    			dbg_printf("\r\nDisplayPort Main Link Active");
				dbg_printf("\r\n  > Config: ");
				if(aux_data[0]==0x14){
					dbg_printf("Link BW & Count -> 5.4 Gbps;");
				}
				else if(aux_data[0]==0x0A){
					dbg_printf("Link BW & Count -> 2.7 Gbps;");
				}
				else{
					dbg_printf("Link BW & Count -> 1.62 Gbps;");
				}

				if((aux_data[1]&0x7)==0x04){
					dbg_printf(" 4-lanes");
				}
				else if((aux_data[1]&0x7)==0x02){
					dbg_printf(" 2-lanes");
				}
				else{
					dbg_printf(" 1-lane");
				}
    		}
    		else{
#if PRINT_STATUS
				dbg_printf("\n\r Lane 0 Status: CR Done? %s;Channel EQ Done? %s;Symbol Locked? %s \n\r",
						   ((aux_data1[0]&0x01)==0x01 ? "Y" : "N"),
						   ((aux_data1[0]&0x02)==0x02 ? "Y" : "N"),
						   ((aux_data1[0]&0x04)==0x04 ? "Y" : "N"));
#endif
    		}
    	}
    }

    return status;
}

/*
 *  Function:
 *
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
void dplpmMainLinkEnable(UINT32 enable)
{
    XILDPLinkPolicyMakerData *lpmd = dplpmGetLinkPolicyMakerData();
    // Enable main link video - don't call the toggle function
    lpmd->enable_main_stream = enable;
    // Reset the scrambler
    dptx_reg_write(XILINX_DISPLAYPORT_TX_FORCE_SCRAMBLER_RESET, 0x01);
    // Enable the main stream
    dptx_reg_write(XILINX_DISPLAYPORT_TX_ENABLE_MAIN_STREAM, lpmd->enable_main_stream);
    // Turn off blanking
}


/*
 *  Function: 
 *      
 *
 *  Parameters:
 *		None
 *
 *  Returns:
 *      None
 *
 *  Example:
 *      
 *
 */
void dplpmTogglePowerMode(void)
{
    XILDPLinkPolicyMakerData *lpmd = dplpmGetLinkPolicyMakerData();

    // Get the current power state
    xildpAUXRead(XILINX_DISPLAYPORT_DPCD_SET_POWER, 1, &lpmd->pm);
    // Toggle it now
    if (lpmd->pm == XILINX_DISPLAYPORT_POWER_STATE_ON)
        lpmd->pm = XILINX_DISPLAYPORT_POWER_STATE_PWRSAVE;
    else
        lpmd->pm = XILINX_DISPLAYPORT_POWER_STATE_ON;
    xildpAUXWrite(XILINX_DISPLAYPORT_DPCD_SET_POWER, 1, &lpmd->pm);
    dbg_printf("Power State: %s\n\r", lpmd->pm == 0x02 ? "Power down" : "Active");
}




/*
 *  Function: 
 *      
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
UINT32 dplpmVerifyLinkStatus(void)
{
    UINT32 status = 0, link_ok = 0;
    volatile UINT8 aux_data[8];

    // Make sure HPD is asserted
    if ( (xildpGetHPDStatus() & 0x01) != 0x01 )
    {
        // Shut off main link
        dplpmMainLinkEnable(0);
        status = XILINX_DISPLAYPORT_NOT_CONNECTED;
    }
    else
    {
        //dbg4_printf("Getting Sink count\n\r");
    	status = xildpAUXRead(XILINX_DISPLAYPORT_DPCD_SINK_COUNT, 8, &aux_data);
        if (!status){
            status = (aux_data[4] & 0x01) << 16 | aux_data[3] << 8 | aux_data[2];
            //dbg1_llc_printf("Getting Sink Lane Status %x\n\r",status);
        } else {
        	dbg_printf("Getting Sink Failed - Aux status %x!! Trying again..\n\r",status);
        	status = xildpAUXRead(XILINX_DISPLAYPORT_DPCD_SINK_COUNT, 8, &aux_data);
            if (!status){
                status = (aux_data[4] & 0x01) << 16 | aux_data[3] << 8 | aux_data[2];
                dbg1_llc_printf("Getting Sink Lane Status %x\n\r",status);
            } else {
            	dbg_printf("Getting Sink Failed - Aux status %x!! No more retry..\n\r",status);
            }
        }

        switch (dptx_reg_read(XILINX_DISPLAYPORT_TX_LANE_COUNT_SET))
        {
            case 0:
                // Lane count not set
                link_ok = 0xF3000000;
                break;
            case 1:
                link_ok = 0x00010007;
                break;
            case 2:
                link_ok = 0x00010077;
                break;
            case 4:
                link_ok = 0x00017777;
                break;
        }
    
        if ((status & link_ok) == link_ok)
            status = XILINX_DISPLAYPORT_OPERATION_SUCCESS;
        else
            status = XILINX_DISPLAYPORT_LPM_STATUS_RETRAIN;
    }
    // Return operational status

    dbg4_printf("Verify done\n\r");
    return status;
}

/*
 *  Function: 
 *      
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
UINT32 dplpmChangeLinkRate(void)
{
    XILDPMainLinkConfig *mlconf = dplpmGetMainLinkConfig();
    if (mlconf->link_rate == 0x0A)
        mlconf->link_rate = 0x06;
    else
        mlconf->link_rate = 0x0A;
    xildpSetLinkRate(mlconf->link_rate);
    dbg_printf("Link rate set to %s speed\n\r", mlconf->link_rate == 0x0A ? "HIGH" : "LOW");
    return mlconf->link_rate;
}

/*
 *  Function: 
 *      
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
UINT32 dplpmChangeLaneCount(void)
{
    XILDPMainLinkConfig *mlconf = dplpmGetMainLinkConfig();
    switch (mlconf->lane_count & 0x0F)
    {
        case 1:
            mlconf->lane_count = 2;
            break;
        case 2:
            mlconf->lane_count = 4;
            break;
        case 4:
            mlconf->lane_count = 1;
            break;
        default:
            mlconf->lane_count = 1;
            break;
    }
    dbg_printf("Lane count set to %d\n\r", mlconf->lane_count);
    xildpSetLaneCount(mlconf->lane_count);
    return mlconf->lane_count;
}

/*
 *  Function: 
 *      
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


UINT32 dplpmReadEDID(XILDP_EDID_Info *ei, UINT32 write_checksum)
{
    UINT32 status = 0;
    UINT8 edid_data[128];
    //UINT8 aux_data[2];
    XILDPMainLinkConfig *mlconf = dplpmGetMainLinkConfig();

   if (mlconf->edid_disable == 1 ) {
        dbg_printf("EDID Reads skipped - use \'e\' to enable/disable\n\r");
        return XILINX_DISPLAYPORT_OPERATION_FAILED;
   }


    // Read EDID over Displayport (I2C-over-AUX)
    status = xildpReadEDID(edid_data);
    if ( status )
    {
        dbg_printf("DPORT: EDID I2C-over-AUX channel read failed (0x%04x)\n\r", status);
        return XILINX_DISPLAYPORT_OPERATION_FAILED;
    }

    if (write_checksum)
    {

        dbg_printf("EDID Write test response check sum %x\n\r", edid_data[127] );
        xildpAUXWrite(XILINX_DISPLAYPORT_DPCD_TEST_EDID_CHECKSUM, 1, &edid_data[127]);

    }

    return status;
}

/*
 *  Function: 
 *      
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
UINT32 dplpmGetPreferredTiming(XILDP_EDID_Info *ei)
{
    UINT32 xx = 0, dm = 0;

    // Check each block for preferred timing mode
    for (xx = 0; xx < XIL_EDID_MAX_DTD_BLOCKS; xx++)
    {
        // Found a preferred timing mode
        if(ei->dtd_blocks[xx].dtd_type == EDID_DTD_DETAILED_TIMING)
        {
            // Get the closest supported mode
            //dm = modetable_get_compatible_mode(ei->dtd_blocks[xx].h_active, ei->dtd_blocks[xx].v_active, (ei->dtd_blocks[xx].pixel_clock * 1000));
            xx = XIL_EDID_MAX_DTD_BLOCKS;
            break;
        }
    }

    // If no preferred mode, default to SVGA
    if (dm == 0)
        dm = VIDEO_MODE_800_600_60_32;
        
    return dm;
}

/*
 *  Function: 
 *      
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
UINT32 dplpmConfigureLink(UINT32 failsafe, UINT32 test_enable, UINT8 read_edid)
{
    UINT32 status = 0;
    XILDPMainLinkConfig *mlconf = dplpmGetMainLinkConfig();

    // If link is being reconfigured, shut down main link
    dplpmMainLinkEnable(0);
    // Check for the failsafe mode
    if (failsafe)
        mlconf->failsafe = 1;
    else
        mlconf->failsafe = 0;

    // Read DPCD capabilities
    status = xildpGetSinkCaps(&mlconf->sink_dpcd);
    if (mlconf->use_dpcd_caps)
    {
        // If the DPCD read succeeded...
        if (!status)
        {
            // Use those values
//        	xil_printf("\r\n Read DPCD succeeded \r\n");
            mlconf->lane_count = (mlconf->sink_dpcd.max_lane_count & 0x07);
            mlconf->link_rate = mlconf->sink_dpcd.max_link_speed;
        }
        else
        {
            // Otherwise default to max settings
        	xil_printf("\r\n DPCD read failed. Default to max settings");
            mlconf->lane_count = 4;
            mlconf->link_rate = XILINX_DISPLAYPORT_LINK_RATE_5_4GBPS;
        }
        dbg_printf("DPCD: %d lanes, 0x%02x speed\n\r", mlconf->lane_count, mlconf->link_rate);
    }

    if ( read_edid ) {
        // Read the EDID
        status = dplpmReadEDID(&mlconf->ei, test_enable);
    }
    // Check for preferred mode use
    if (mlconf->use_preferred_mode) {
        mlconf->video_mode_id = dplpmGetPreferredTiming(&mlconf->ei);
    }
    // Check for EDID corruption
    if (status == XILINX_DISPLAYPORT_EDID_CORRUPT)
    {
        dbg_printf("EDID Corruption Detected.\n\r");
        mlconf->video_mode_id = 0;//VIDEO_MODE_800_600_60_32;
        mlconf->failsafe = 1;
    }


    dptx_reg_write(XILINX_DISPLAYPORT_TX_ENHANCED_FRAME_EN, mlconf->efm);

    // Set the link rate
    xildpSetLinkRate(mlconf->link_rate);
    // Set lane count
    xildpSetLaneCount(mlconf->lane_count);

    // Set downspread control
    xildpSetDownspread(mlconf->ssc);

    mlconf->video_mode_update = 1;

    return status;
}

/*
 *  Function: 
 *      
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
XILDPMainLinkConfig *dplpmGetMainLinkConfig(void)
{
    static XILDPMainLinkConfig main_link_config;
    return &main_link_config;
}

/*
 *  Function: 
 *      
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
XILDPLinkPolicyMakerData *dplpmGetLinkPolicyMakerData(void)
{
    static XILDPLinkPolicyMakerData link_policy_maker_data;
    return &link_policy_maker_data;
}

UINT32 dplpmCheckHPDStatus(void)
{
    // Read these registers continually
    volatile UINT32 hpd = 0;
    volatile UINT32 hpd_intr = 0;
    volatile UINT32 hpd_duration = 0;
    volatile UINT32 hpd_pulse_detected = 0;//[4]th bit of 0x140

    static DisplayportHPDState hpd_state = HPD_STATE_DISCONNECTED;
    UINT32 status = 0;

    //Reading raw state of HPD
    hpd = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT_SIG_STATE) & XILINX_DISPLAYPORT_TX_AUX_HPD;

    //hpd_intr[0] -> An IRQ on HPD signal detected;
    //hpd_intr[1] -> Detected presence of HPD; Interrupt asserts immediately after HPD detection and after HPD loss for 2ms
    //hpd_intr[4] -> HPD pulse detected;check 0x150 for duration
    hpd_intr = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT);
    hpd_pulse_detected = hpd_intr & 0x10;

    if ( (  hpd_intr & 0x11 ) == 0x10 ) { //HPD event detected
    	hpd_duration = dptx_reg_read(XILINX_DISPLAYPORT_TX_AUX_TRANSACTION_STATUS+0x4);
    }

    switch (hpd_state)
    {
        case HPD_STATE_DISCONNECTED:
            if (hpd == 1 || (hpd_pulse_detected == 0x10)) { // if hot plug detect
            	dbg_printf("\r\nDetected Connection Event");
            	dbg_printf("\r\nTraining in Progress...");
                hpd_state = HPD_STATE_CONNECTED;
                dplpmMaintainLink(1,1);  // link training
            }
            break;
        case HPD_STATE_CONNECTED:
        	if (hpd == 0) { //Look for disconnect event
            	if ((hpd_intr & 0x3) == 0x02) {
					dbg_printf("\r\nDetected Disconnection Event");
					hpd_state = HPD_STATE_DISCONNECTED;
					dplpmMainLinkEnable(0);
            	}
        	}
        	else {
            	if (hpd_pulse_detected == 0x10) {
                	hpd_duration = dptx_reg_read(XILINX_DISPLAYPORT_TX_AUX_TRANSACTION_STATUS+0x4);
                	if ( hpd_duration > 250) {
                    	status = dplpmVerifyLinkStatus();
                    	if ( status == XILINX_DISPLAYPORT_LPM_STATUS_RETRAIN ) { // the link is not trained, then train it
                            hpd_state = HPD_STATE_INTERRUPT;
                        	dbg_printf("\r\nDetected HPD irq (Duration = %d us)", hpd_duration);
                    	}
                	}
            	}
        	}
        	break;
        case HPD_STATE_INTERRUPT:
        	// check link status and train as required.
			status = dplpmVerifyLinkStatus();
			if ( status == XILINX_DISPLAYPORT_LPM_STATUS_RETRAIN ) { // the link is not trained, then train it
				dbg_printf("\r\nProcessing Interrupt from Sink...");
				xildpResetPHYTX(XILINX_DISPLAYPORT_PHY_RESET_MASTER);
				hpd_state = HPD_STATE_CONNECTED;
				dplpmMaintainLink(1,1);
			}
            break;
        default:
            // Fall back to disconnected state if the machine gets hosed
            hpd_state = HPD_STATE_DISCONNECTED;
            dplpmMainLinkEnable(0);
            break;
    }

    return hpd_state;
}

/*
 *  Function: 
 *      
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
UINT32 dplpmMaintainLink(UINT32 force_retrain, UINT8 read_edid)
{
    UINT32 max_training_attempts = 5;
    UINT32 training_successful = 0, retrain = 0;
    UINT32 status = XILINX_DISPLAYPORT_OPERATION_SUCCESS;
    XILDPMainLinkConfig *mlconf = dplpmGetMainLinkConfig();
    XILDPLinkPolicyMakerData *lpmd = dplpmGetLinkPolicyMakerData();


#if PRINT_TS
    UINT32 start = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT + 0x14);
#endif

    retrain = force_retrain;
    // Turn off the main link
    dplpmMainLinkEnable(0);
    if ( !force_retrain ){
        status = dplpmVerifyLinkStatus();
        if (status == XILINX_DISPLAYPORT_LPM_STATUS_RETRAIN)
            retrain = 1;
    }


    // Check to see if link needs to be retrained
    if (retrain == 1)
    {

       // Toggle the power state
       xildpSetPowerState(XILINX_DISPLAYPORT_POWER_STATE_PWRSAVE);
       MB_Sleep(1);
       //xildpWaituS(510);
       xildpSetPowerState(XILINX_DISPLAYPORT_POWER_STATE_ON);
       MB_Sleep(1);
       //xildpWaituS(510);

        // Reconfigure the link
        dplpmConfigureLink(0, 0, (!mlconf->edid_disable));

        // Make up to 'max_training_attempts' to train the link
        while (max_training_attempts > 0 && training_successful != 0x01)
        {
            // Attempt to train the link
            status = dplpmTrainLink(mlconf->training_settings, lpmd->preserve_linkrate, 0);
            //dbg_llc_printf("%d Status : 0x%08x\n\r", max_training_attempts, status);
            status = dplpmVerifyLinkStatus();
            // Verify link status
            if (status == XILINX_DISPLAYPORT_OPERATION_SUCCESS)
            {
                // Set the training successful flag
                training_successful = 0x01;
                // Set the video mode
                //vcmSetVideoMode(mlconf->video_mode_id, mlconf->failsafe, vcmGetFrameBuffer());
                mlconf->video_mode_update = 1;
                // Enable main link video
                dplpmMainLinkEnable(1);
            }
            else // Decrement training attempts
                max_training_attempts--;
        }
    } else  {
    	// Turn on the main link, no need to re-train
    	dplpmMainLinkEnable(1);
    }


#if PRINT_TS
    UINT32 end = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT + 0x14);
    UINT32 diff = end - start;
    if ( start > end ) diff = start - end;
    dbg2_llc_printf("LT takes %0d us\n\r", diff);
#endif
    // display status of the link training..
    dplpmReadStatus();
    return status;
}


