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


#include "xil_displayport.h"
#include "displayport_defs.h"
#include "sys_defs.h"
#include "xio.h"
#include "xtmrctr.h"
#include "xparameters.h"
#include "stdio.h"
#include "string.h"
#include "xlib_string.h"
#include "displayport_lpm.h"
#include "displayport_tx_drv.h"
#include "microblaze_sleep.h"


#define XILINX_DISPLAYPORT_MAX_TRAINING_ATTEMPTS 5

// Strings used by printout functions
char *dpcd_strings[] =
{
    "Unknown",         // 0
    "Invalid",         // 1
    "None",            // 2
    "Yes",             // 3
    "No",              // 4
    "1.0",             // 5
    "1.1",             // 6
    "1.62 Gbps",       // 7
    "2.7 Gbps",        // 8
    "DisplayPort",     // 9
    "Analog VGA",      // 10
    "DVI",             // 11
    "HDMI",            // 12
    "Other (No EDID)", // 13
    "Rev 1.2",           //14
    "5.4 Gbps"
};

// Globals
UINT8 preset_vswing_tx, preset_vswing_rx;
UINT8 preset_preemp_tx, preset_preemp_rx;
UINT8 adj_req_lane01, adj_req_lane23;

/*
 *  Function: xildpInitDisplayport
 *      Initialize the Displayport cores (TX and RX) if present
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      Zero on sucess, non-zero on failure
 * 
 *  Example:
 *  (start code)
 *      UINT32 status = xildpInitDisplayport();
 *  (end)
 *      
*/ 
UINT32 xildpInitDisplayport(void)
{
    UINT32 status = XILINX_DISPLAYPORT_OPERATION_SUCCESS;
    UINT32 core_type = xildpGetCoreID();

    status = (core_type << 16);

    switch (core_type)
    {
        case XILINX_DISPLAYPORT_DEVICE_TYPE_TRANSMITTER:
            status |= (dptx_init() & 0xFF);
            break;
        default:
            status = XILINX_DISPLAYPORT_OPERATION_FAILED;
            break;
    }
    // Initialize the presets
    xildpPresetVSwing(XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
    xildpPresetPreemphasis(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);

    // Status code is  device type and core status bytes
    //    device_type << 16 | RX status << 8 | TX status
    return status;
}

/*
 *  Function: xildpGetCoreID
 *      Get the core ID(s) of any installed DisplayPort cores
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      Logical OR of the ID codes for transmitter and receiver cores. ID codes are as follows:
 *         o XILINX_DISPLAYPORT_DEVICE_TYPE_TRANSMITTER
 *         o XILINX_DISPLAYPORT_DEVICE_TYPE_RECEIVER
 * 
 *	Example:
 *  (start code)
 *      UINT32 core_id = xildpGetCoreID();
 *  (end)
 * 
*/ 
UINT32 xildpGetCoreID(void)
{
    UINT32 core_type = 0;
    if ((dptx_reg_read(XILINX_DISPLAYPORT_TX_CORE_ID) & 0xFF ) == XILINX_CORE_ID_DISPLAYPORT_TX)
        core_type |= XILINX_DISPLAYPORT_DEVICE_TYPE_TRANSMITTER;

    return core_type;
}

/*
 *  Function: xildpAUXRead
 *      Generate an AUX READ request for the specified address and number of bytes
 *
 *	Parameters:
 *     address    - : destination address to read
 *     byte_count - : number of bytes to read
 *     data       - : array to hold read data bytes (must be allocated)
 * 
 *  Returns:
 *      Status code which is one of the following:
 *               o  XILINX_DISPLAYPORT_OPERATION_SUCCESS  
 *               o  XILINX_DISPLAYPORT_OPERATION_FAILED   
 *               o  XILINX_DISPLAYPORT_OPERATION_DEFER    
 *               o  XILINX_DISPLAYPORT_INVALID_REPLY_CODE 
 *               o  XILINX_DISPLAYPORT_OPERATION_TIMEOUT  
 * 
 *  Example:
 *  (start code)
 *      UINT8 aux_data[16];
 *      UINT32 status = xildpAUXRead(XILINX_DISPLAYPORT_DPCD_REVISION, 12, aux_data);
 *  (end)
 *      
*/
UINT32 xildpAUXRead(UINT32 address, UINT32 byte_count, void* data)
{
    UINT32 status = XILINX_DISPLAYPORT_OPERATION_SUCCESS, xx = 0;
    XILDPAUXTransaction request;
    XILDPMainLinkConfig *mlconf = dplpmGetMainLinkConfig();
    UINT8 *rd_data = (UINT8*) data;

#if PRINT_TS
   UINT32 start, end, diff;
   start = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT + 0x14);
#endif

   // dbg3_printf("AUX Read\n\r");
    if (data == NULL)
    {
        dbg_printf("Pointer for storage of read data is NULL.\n\r");
        return XILINX_DISPLAYPORT_OPERATION_FAILED;
    }

    if (xildpGetHPDStatus() != 0x01)
    {
        dbg_printf("No Displayport sink device connected. Ignored.\n\r");
        return XILINX_DISPLAYPORT_NOT_CONNECTED;
    }

    //dbg3_printf("HPD GOOD\n\r");

    // Setup command
    request.cmd_code  = XILINX_DISPLAYPORT_CMD_REQUEST_READ;
    request.address   = address;
    request.num_bytes = byte_count;
    if ( mlconf->aux_log_enable ) {
       dbg_printf("AUX Read : Address %04x, Bytes %02x, Data\r\n\t", address, byte_count);
    }

    status = xildpAUXSubmitCommand(&request);


#if PRINT_TS
    end = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT + 0x14);
    diff = end - start;
    if ( start > end ) diff = start - end;
#endif

    if (status != XILINX_DISPLAYPORT_CMD_REPLY_ACK)
    {
#if PRINT_TS
    	dbg1_llc_printf("Failed!.. %0d\n\r",diff);
#else
    	dbg1_llc_printf("Failed!..\n\r");
#endif
        return XILINX_DISPLAYPORT_OPERATION_FAILED;
    }
    else
    {
        for (xx=0; xx < byte_count; xx++)
            rd_data[xx] = dptx_reg_read(XILINX_DISPLAYPORT_TX_RCV_FIFO);

        if ( mlconf->aux_log_enable ) {
            for (xx=0; xx < byte_count; xx++) {
            	dbg_printf("0x%02x\t", rd_data[xx]);
            }
            dbg_printf("\n\r");
        }

#if PRINT_TS
       dbg1_llc_printf("Done..%0d\n\r", diff);
#endif
    }
    //dbg3_printf("AUX Read done\n\r");

    MB_Sleep(1);
    //xildpWaituS(1);

    return status;
}
/*
 *  Function: xildpAUXWrite
 *      Generate an AUX READ request for the specified address and number of bytes
 *
 *	Parameters:
 *      address    - : destination address to be writen
 *      byte_count - : number of bytes to be writen
 *      data       - : array containing the data to be written
 * 
 *  Returns:
 *      Status code which is one of the following:
 *               o  XILINX_DISPLAYPORT_OPERATION_SUCCESS  
 *               o  XILINX_DISPLAYPORT_OPERATION_FAILED   
 *               o  XILINX_DISPLAYPORT_OPERATION_DEFER    
 *               o  XILINX_DISPLAYPORT_INVALID_REPLY_CODE 
 *               o  XILINX_DISPLAYPORT_OPERATION_TIMEOUT  
 * 
 *  Example:
 *  (start code)
 *      UINT8 aux_data[16];
 *      UINT32 status = xildpAUXWrite(XILINX_DISPLAYPORT_DPCD_LINK_BANDWIDTH_SET, 7, aux_data);
 *  (end)
 *      
*/
UINT32 xildpAUXWrite(UINT32 address, UINT32 byte_count, void* data)
{
    UINT32 status = XILINX_DISPLAYPORT_OPERATION_SUCCESS;
    XILDPAUXTransaction request;
    XILDPMainLinkConfig *mlconf = dplpmGetMainLinkConfig();
#if PRINT_TS
    UINT32 start, end, diff;
    start = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT + 0x14);
#endif

    if (data == NULL)
    {
       // dbg_printf("Pointer to write data is NULL.\n\r");
        return XILINX_DISPLAYPORT_OPERATION_FAILED;
    }

    if (xildpGetHPDStatus() != 0x01)
    {
        dbg_printf("No Displayport sink device connected. Ignored.\n\r");
        return XILINX_DISPLAYPORT_NOT_CONNECTED;
    }

    // Setup command
    request.cmd_code  = XILINX_DISPLAYPORT_CMD_REQUEST_WRITE;
    request.address   = address;
    request.num_bytes = byte_count;
    request.wr_data   = (UINT8*)data;

    UINT8 iter;
    UINT8 *wrptr = (UINT8*)data;
    if ( mlconf->aux_log_enable ) {
    	dbg_printf("AUX Write: Address %04x, Bytes %02x, Data\t", address, byte_count);
    }


    status = xildpAUXSubmitCommand(&request);
#if PRINT_TS
    end = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT + 0x14);
    diff = end - start;
    if ( start > end ) diff = start - end;
#endif
    if (status)
    {
        //printf("ERROR: AUX Write to address 0x%04x failed with code 0x%04x.\n\r", address, status);
        status = XILINX_DISPLAYPORT_OPERATION_FAILED;
#if PRINT_TS
        dbg1_llc_printf("Failed..%d\n\r",diff);
#else
        dbg1_llc_printf("Failed..\n\r");
#endif
    }
    else {
        if ( mlconf->aux_log_enable ) {
        	for ( iter = 0; iter < byte_count; iter++ ) {
            	dbg_printf("0x%02x\t", wrptr[iter]);
        	}
        	dbg_printf("\n\r");
        }


#if PRINT_TS
        dbg1_llc_printf("Done..%d\n\r",diff);
#endif
    }

    MB_Sleep(1);
    //xildpWaituS(1000);

    return status;
}

/*
 *  Function: xildpGetHPDStatus
 *      Returns the current status of the Hot Plug Detect (HPD) register
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      '1' indicates HPD is actvie (asserted), '0' indicates deasserted
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpGetHPDStatus();
 *  (end)
 *      
*/ 
inline UINT32 xildpGetHPDStatus(void)
{
//dbg3_printf("Getting HPD Status\n\r");
	return (dptx_reg_read(XILINX_DISPLAYPORT_TX_STATUS) & XILINX_DISPLAYPORT_TX_AUX_HPD);
}

/*
 *  Function: xildpWaituS
 *      Wait loop for timed operations. 
 *
 *	Parameters:
 *      us - : time to wait in microseconds
 * 
 *  Returns:
 *      None
 * 
 *	Example:
 *  (start code)
 *      xildpWaituS(5 * SYSDEF_ONE_MICROSECOND);
 *  (end)
 *      
*/ 
void xildpWaituS(UINT32 us)
{

  extern XTmrCtr TimerCounterInst;

  //dbg_printf(".");
    // Reset and start timer
    XTmrCtr_Start(&TimerCounterInst, 0);
    
    // Wait
//    xil_printf("[TIMER] <<<< Start = %d\n\r", XTmrCtr_GetValue(&TimerCounter, 0));
    while( XTmrCtr_GetValue(&TimerCounterInst, 0) < ( us * ( /*XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ*/50000000 / 1000000 ) ) );
//    xil_printf("[TIMER]  Stop >>>> = %d\n\r", XTmrCtr_GetValue(&TimerCounter, 0));
    // Stop timer
    XTmrCtr_Stop(&TimerCounterInst, 0);
}

/*
 *  Function: xildpReadEDID
 *      Read EDID data for the device connected via Displayport 
 *
 *	Parameters:
 *      edid_data - : array to hold EDID data (must be previously allocated, 128 byte minimum)
 * 
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT8 edid_data[128];
 *      UINT32 status = xildpReadEDID(&edid_data[0]);
 *  (end)
 *      
*/ 
UINT32 xildpReadEDID(UINT8 *edid_data)
{
    UINT32 status = 0, xx = 0;

    // Make sure HPD is asserted
    if ( xildpGetHPDStatus() != 0x01 )
        return XILINX_DISPLAYPORT_NOT_CONNECTED;

    for(xx = 0; xx < 127; xx+=16)
    {
        status = xildpI2CRead(0x50, xx, 16, &edid_data[xx], (xx==0), (xx==112), 0);
        //if (dptx_reg_read(XILINX_DISPLAYPORT_TX_ENABLE_MAIN_STREAM) == 0x01)
        //    dbg_printf(".");
        if (status)
            break;
    }
#if PRINT_EDID
    if ( !status ) {
        dbg_printf("\n\r========================== Reading EDID... Start ========================= \n\r");
        for(xx = 0; xx < 127; xx+=16){
            dbg_printf("EDID[%03d to %03d] %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, "
            		                      "%02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x\n\r",
            		xx, xx+16,
            		edid_data[xx],   edid_data[xx+1], edid_data[xx+2], edid_data[xx+3],
            		edid_data[xx+4], edid_data[xx+5], edid_data[xx+6], edid_data[xx+7],
            		edid_data[xx+8], edid_data[xx+9], edid_data[xx+10],edid_data[xx+11],
            		edid_data[xx+12],edid_data[xx+13],edid_data[xx+14],edid_data[xx+15]);
        }
        dbg_printf("========================== Reading EDID...Done ==========================\n\r\n\r");
    } else {
    	dbg_printf("Reading EDID Failed...\n\r");
    }
#endif

    return status;
}

/*
 *  Function: xildpDumpDPCD
 *      Print out the entire contents of the DPCD for the attached sink devise. Does
 *      not include training configuration or lane status.
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      None
 * 
 *	Example:
 *  (start code)
 *      xildpDumpDPCD();
 *  (end)
 *      
*/ 
void xildpDumpDPCD(void)
{
    UINT32 xx = 0;
    UINT32 status = 0;
    XILDPDPCDData dpcd;
    if (xildpGetHPDStatus() != 0x01)
    {
//        dbg_printf("No Displayport sink device connected. Ignored.\n\r");
        return;
    }

    status = xildpGetSinkCaps(&dpcd);
    if (status)
        dbg_printf("Error: failed to get sink capabilities.\n\r");

    // Format and printout the data;
    dbg_printf("Displayport Configuration Data:\n\r"
           "   DPCD Revision                 : %s\n\r"
           "   Max Link Rate                 : %s\n\r"
           "   Max Lane Count                : %d\n\r"
           "      Enhanced Framing           : %s\n\r"
           "   Max Downspread                : %s\n\r"
           "      Require AUX Handshake      : %s\n\r"
           "   Number of RX Ports            : %d\n\r"
           "   Main Link ANSI 8B/10B         : %s\n\r"
           "   Downstream Port Count         : %d\n\r"
           "      Format Conversion Support  : %s\n\r"
           "      OUI Support                : %s\n\r"
           "   Receiver Port 0:\n\r"
           "     Has EDID                    : %s\n\r"
           "     Uses Previous Port          : %s\n\r"
           "     Buffer Size                 : %d\n\r",
            dpcd.rev_string,
            dpcd.link_rate_string,
            dpcd.max_lane_count,
           (dpcd.enhanced_framing_support == TRUE) ? dpcd_strings[3] : dpcd_strings[4],
           (dpcd.downspread_support == TRUE) ? "0.5%" : "None",
           (dpcd.require_aux_handshake == TRUE) ? dpcd_strings[4] : dpcd_strings[3],
            dpcd.num_rcv_ports,
           (dpcd.ansi_8B10_support == TRUE) ? dpcd_strings[3] : dpcd_strings[4],
            dpcd.num_downstream_ports,
           (dpcd.format_conversion_support == TRUE) ? dpcd_strings[3] : dpcd_strings[4],
           (dpcd.oui_support == TRUE)  ? dpcd_strings[3] : dpcd_strings[4],
           (dpcd.rx0_has_edid == TRUE) ? dpcd_strings[3] : dpcd_strings[4],
           (dpcd.rx0_use_prev == TRUE) ? dpcd_strings[3] : dpcd_strings[4],
            dpcd.rx0_buffer_size);

    for (xx = 0; xx < dpcd.num_downstream_ports; xx++)
    {
    dbg_printf("   Downstream Port %d:\n\r"
               "      Port Type         : %s\n\r"
               "      HPD Aware         : %s\n\r",
               xx, dpcd.downstream_port_types[xx],
               ((dpcd.downstream_port_caps[xx] & 0x08) == 0x08) ? dpcd_strings[3] : dpcd_strings[4] );
    }
}

/*
 *  Function: xildpGetSinkCaps
 *      Retrieve the DPCD capabilities fields of the attached sink device
 *
 *	Parameters:
 *      dpcd_data - : pointer to a struct to hold the DPCD configuration data
 * 
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *  Example:
 *  (start code)
 *      XILDPDPCDData sink_caps;
 *      UINT32 status = xildpGetSinkCaps(&sink_caps);
 *  (end)
 *      
*/ 
UINT32 xildpGetSinkCaps(XILDPDPCDData *dpcd_data)
{
    UINT32 status = 0, xx = 0;
    XILDPDPCDData *sink_info = dpcd_data;
    UINT8 aux_read_data[16];

    // If a valid struct pointer is passed in, fill out that struct instead
    if (dpcd_data != NULL)
        sink_info = dpcd_data;
    else
    {
        dbg_printf("Invalid data pointer for DPCD info.\n\r");
        return XILINX_DISPLAYPORT_OPERATION_FAILED;
    }

    status = xildpAUXRead(XILINX_DISPLAYPORT_DPCD_REVISION, 0x0C, aux_read_data);

    //for(xx = 0; xx<0x0C; xx++){
    //	dbg_printf("AUX Reg[%d] 0x%08x\n\r",xx,aux_read_data[xx]);
    //}

    if (status)
        return XILINX_DISPLAYPORT_OPERATION_FAILED;

    // Clear data structures
    memset(sink_info->downstream_port_types, 0, 16);
    memset(sink_info->downstream_port_caps, 0, 16);

    // Major / Minor revision
    switch (aux_read_data[0])
    {
        case 0x10:
            sink_info->dpcd_rev = 0x10;
            sink_info->rev_string = dpcd_strings[5];
            break;
        case 0x11:
            sink_info->dpcd_rev = 0x11;
            sink_info->rev_string = dpcd_strings[6];
            break;
        case 0x12:
            sink_info->dpcd_rev = 0x12;
            sink_info->rev_string = dpcd_strings[14];
            break;
        default:
            sink_info->dpcd_rev = 0x00;
            sink_info->rev_string = dpcd_strings[0];
            break;
    }

    // Link Rate
    switch (aux_read_data[1])
    {
        case XILINX_DISPLAYPORT_LINK_RATE_1_62GBPS:
            sink_info->max_link_speed = XILINX_DISPLAYPORT_LINK_RATE_1_62GBPS;
            sink_info->link_rate_string = dpcd_strings[7];
            break;
        case XILINX_DISPLAYPORT_LINK_RATE_2_7GBPS:
            sink_info->max_link_speed = XILINX_DISPLAYPORT_LINK_RATE_2_7GBPS;
            sink_info->link_rate_string = dpcd_strings[8];
            break;
        case XILINX_DISPLAYPORT_LINK_RATE_5_4GBPS:
            sink_info->max_link_speed = XILINX_DISPLAYPORT_LINK_RATE_5_4GBPS;
            sink_info->link_rate_string = dpcd_strings[15];
            break;
        default:
            sink_info->max_link_speed = 0;
            sink_info->link_rate_string = dpcd_strings[1];
            break;
    }

    // Lane count
    sink_info->max_lane_count = aux_read_data[2] & 0x07;
    // Enhanced framing support
    sink_info->enhanced_framing_support = (aux_read_data[2] & 0x80) >> 7;
    // Max downspread support
    sink_info->downspread_support = (aux_read_data[3] & 0x01);
    // AUX handshake required
    sink_info->require_aux_handshake = (aux_read_data[3] & 0x40) >> 6;
    // Number of receive ports
    sink_info->num_rcv_ports = (aux_read_data[4] & 0x01) + 1;
    // ANSI 8B/10B coding support
    sink_info->ansi_8B10_support = (aux_read_data[6] & 0x01);
    // Number of downstream ports
    sink_info->num_downstream_ports = (aux_read_data[7] & 0x0F);
    sink_info->oui_support = (aux_read_data[7] & 0x80) >> 7;
    // Receiver Port 0 capabilities
    sink_info->rx0_has_edid = (aux_read_data[8] & 0x02);
    sink_info->rx0_use_prev = (aux_read_data[8] & 0x04);
    sink_info->rx0_buffer_size = aux_read_data[9];

    if (sink_info->num_downstream_ports)
    {
        // Check for format conversion support
        sink_info->format_conversion_support = (aux_read_data[5] >> 4) & 0x01;
        sink_info->oui_support = (aux_read_data[7] >> 8) & 0x01;
        // Read downstream port capabilities
        xildpAUXRead(XILINX_DISPLAYPORT_DPCD_DOWNSTREAM_PORT_CAPS, sink_info->num_downstream_ports, aux_read_data);
        // Copy to local array
        memcpy(sink_info->downstream_port_caps, aux_read_data, sink_info->num_downstream_ports);
        // Get the port type
        for (xx = 0; xx < sink_info->num_downstream_ports; xx++)
        {
            // Downstream port type
            switch ((sink_info->downstream_port_caps[xx] & 0x07))
            {
                case 0x01:
                    sink_info->downstream_port_types[xx] = dpcd_strings[9];
                    break;
                case 0x02:
                    sink_info->downstream_port_types[xx] = dpcd_strings[10];
                    break;
                case 0x03:
                    sink_info->downstream_port_types[xx] = dpcd_strings[11];
                    break;
                case 0x04:
                    sink_info->downstream_port_types[xx] = dpcd_strings[12];
                    break;
                case 0x05:
                    sink_info->downstream_port_types[xx] = dpcd_strings[13];
                    break;
                default:
                    sink_info->downstream_port_types[xx] = dpcd_strings[1];
                    break;
            }
        }
    }
    else // No downstream ports
        sink_info->port_type_string = "None";

    return status;
}

/*
 *  Function: xildpSetLinkRate
 *      Set the link rate for the source and sink devices
 *
 *	Parameters:
 *      link_rate  - : link rate to set for main streams
 *                 o   Must be one of the following values:
 *                 o   XILINX_DISPLAYPORT_LINK_RATE_1_62GBPS
 *                 o   XILINX_DISPLAYPORT_LINK_RATE_2_7GBPS
 * 
 *  Returns:
 *      Zero on sucess, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpSetLinkRate(XILINX_DISPLAYPORT_LINK_RATE_1_62GBPS);
 *  (end)
 *      
*/ 
UINT32 xildpSetLinkRate(UINT8 link_rate)
{
    UINT32 status = 0;
    UINT8 tmp[1];

    // Verify link rate
    if (link_rate == 0x0A || link_rate == 0x06 || link_rate == 0x14)
    {
        // Write new link bandwidth value to the receiver
    	//dbg3_printf("Setting Bandwidth to %x\n\r", link_rate);
        status = xildpAUXWrite(XILINX_DISPLAYPORT_DPCD_LINK_BANDWIDTH_SET, 1, &link_rate);
        xildpAUXRead(XILINX_DISPLAYPORT_DPCD_LINK_BANDWIDTH_SET, 1, tmp);
    	//dbg3_printf("RX Bandwidth set to %x\n\r", tmp[0]);
        // Check for successful write
        if (!status)
        {
            // Check to see if the link rate is already set - change it if it's different
            if (link_rate != dptx_reg_read(XILINX_DISPLAYPORT_TX_LINK_BW_SET))
            {
                switch (link_rate)
                {
                    // Set the clock speed here
                    case 0x0A:
                        dptx_set_clkspeed(XILINX_DISPLAYPORT_TX_PHY_CLK_FB_270);
                        break;
                    case 0x06:
                        dptx_set_clkspeed(XILINX_DISPLAYPORT_TX_PHY_CLK_FB_162);
                        break;
                    case 0x14:
                        dptx_set_clkspeed(XILINX_DISPLAYPORT_TX_PHY_CLK_FB_540);
                        break;
                }
                // Write new link bandwidth value to the transmitter
                dptx_reg_write(XILINX_DISPLAYPORT_TX_LINK_BW_SET, link_rate);
            }
        }
    }
    else
        status =  XILINX_DISPLAYPORT_INVALID_PARAMETER;
    // Return status
    return status;
}

/*
 *  Function: xildpSetLaneCount
 *      Set the lane count for the source and sink devices
 *
 *	Parameters:
 *      lane_count - : number of lanes to use. Must be 1, 2 or 4.
 * 
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpSetLaneCount(4);
 *  (end)
 *      
*/ 
UINT32 xildpSetLaneCount(UINT32 lane_count)
{
    UINT32 status = XILINX_DISPLAYPORT_OPERATION_SUCCESS;
    UINT8 efm = 0, num_lanes = 0;

    // Mask the proper number of lanes
    num_lanes = lane_count & 0x07;
    // Verify valid lane count
    if (num_lanes == 3 || lane_count > 4)
    {
        // Error condition - not a valid lane count
        status = XILINX_DISPLAYPORT_OPERATION_FAILED;
    }
    else
    {
        // Get the status of enhanced framing mode from the transmitter
        efm = (dptx_reg_read(XILINX_DISPLAYPORT_TX_ENHANCED_FRAME_EN) & 0x01);
        // check for enhanced framing mode
        if (efm)
            num_lanes |= XILINX_DISPLAYPORT_ENHANCED_FRAMING_MODE_BIT;
        // Write new lane count to sink device
        status = xildpAUXWrite(XILINX_DISPLAYPORT_DPCD_LANE_COUNT_SET, 1, &num_lanes);
        // Write lane count to transmitter
        dptx_reg_write(XILINX_DISPLAYPORT_TX_LANE_COUNT_SET, num_lanes);
    }
    return status;
}

/*
 *  Function: xildpSetPowerState
 *      Set the power saving state in the sink device
 *
 *	Parameters:
 *      pwr_state - : set to power-save or normal operations
 *                o  Must be one of the following values:
 *                o  XILINX_DISPLAYPORT_POWER_STATE_ON
 *                o  XILINX_DISPLAYPORT_POWER_STATE_PWRSAVE
 *
 *  Returns:
 *      Zero on success, non-zero on failure
 *
 *	Example:
 *  (start code)
 *      UINT32 status = xildpSetPowerState(XILINX_DISPLAYPORT_POWER_STATE_ON);
 *  (end)
 *
*/
UINT32 xildpSetPowerState(UINT32 pwr_state)
{
    UINT32 status = 0;
    // Mask pwr_state to appropriate range
    UINT8 aux_data = (pwr_state & 0x03);
    // Verify value of pwr_state
    if (aux_data == XILINX_DISPLAYPORT_POWER_STATE_ON || aux_data == XILINX_DISPLAYPORT_POWER_STATE_PWRSAVE)
    {
        // Write the new power state to the sink device
        status = xildpAUXWrite(XILINX_DISPLAYPORT_DPCD_SET_POWER, 1, &aux_data);
    }
    else
        status = XILINX_DISPLAYPORT_INVALID_PARAMETER;

    return status;
}

/*
 *  Function: xildpSetLinkQualityPattern
 *      Set the link quality pattern for error-checking
 *
 *	Parameters:
 *      pattern - : link quality pattern to use. Must be one of the following:
 *              o XILINX_DISPLAYPORT_LINK_QUAL_PATTERN_D10_2
 *              o XILINX_DISPLAYPORT_LINK_QUAL_PATTERN_SYM_ERR
 *              o XILINX_DISPLAYPORT_LINK_QUAL_PATTERN_PRBS7
 *
 *  Returns:
 *      Zero on success, non-zero on failure
 *
 *	Example:
 *  (start code)
 *      UINT32 status = xildpSetLinkQualityPattern(XILINX_DISPLAYPORT_LINK_QUAL_PATTERN_SYM_ERR);
 *  (end)
 *
*/
UINT32 xildpSetLinkQualityPattern(UINT8 pattern)
{
    UINT32 status = 0, pattern_valid = 1;

    switch (pattern)
    {
        case XILINX_DISPLAYPORT_LINK_QUAL_PATTERN_OFF:
        case XILINX_DISPLAYPORT_LINK_QUAL_PATTERN_D10_2:
        case XILINX_DISPLAYPORT_LINK_QUAL_PATTERN_SYM_ERR:
        case XILINX_DISPLAYPORT_LINK_QUAL_PATTERN_PRBS7:
            // Valid patterns
            break;
        default:
            status = XILINX_DISPLAYPORT_INVALID_PARAMETER;
            pattern_valid = 0;
            break;
    }

    if (pattern_valid == 1)
    {
        // Set pattern in transmitter
        dptx_reg_write(XILINX_DISPLAYPORT_TX_TRAINING_PATTERN_SET, ((pattern >> 2) & 0x03));
        // Set for link qual pattern in sink device
        xildpAUXWrite(XILINX_DISPLAYPORT_DPCD_TRAINING_PATTERN_SET, 1, &pattern);
    }

    return status;
}

/*
 *  Function: xildpSetDownspread
 *      Set the spread-spectrum downspread control in the source and sink devices
 *
 *	Parameters:
 *      downspread_on - : enable or disable downspreading. Zero to disable, non-zero to enable
 * 
 *  Returns:
 *      Zero indicates sucess, non-zero indicates failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpSetDownspread(0x01);
 *  (end)
 *      
*/ 
UINT32 xildpSetDownspread(UINT8 downspread_on)
{
    UINT32 status = 0;
    UINT8 enable = 0x00;

    // Set the enable bit
    if (downspread_on > 0)
        enable = XILINX_DISPLAYPORT_DOWNSPREAD_ENABLE_BIT;
    // Set in transmitter
    dptx_reg_write(XILINX_DISPLAYPORT_TX_DOWNSPREAD_CTRL, (enable >> 4));
    // Set in sink device
    status = xildpAUXWrite(XILINX_DISPLAYPORT_DPCD_DOWNSPREAD_CONTROL, 1, &enable);
    // Return status of AUX write
    return status;
}

/*
 *  Function: xildpSetEnhancedFramingMode
 *      Enable or disable enhanced framing mode
 *
 *	Parameters:
 *      efm_enable - : Enable / Disable Enhanced Framing Mode (non-zero enables, zero disables)
 *
 *  Returns:
 *      Zero on success, non-zero on failure
 *
 *	Example:
 *  (start code)
 *
 *  (end)
 *
*/
UINT32 xildpSetEnhancedFramingMode(UINT32 efm_enable)
{
    UINT32 status = 0;
    UINT8 lane_count = dptx_reg_read(XILINX_DISPLAYPORT_TX_LANE_COUNT_SET);

    if (efm_enable)
    {
        dptx_reg_write(XILINX_DISPLAYPORT_TX_ENHANCED_FRAME_EN, 0x01);
        lane_count |= XILINX_DISPLAYPORT_ENHANCED_FRAMING_MODE_BIT;
    }
    else
    {
        dptx_reg_write(XILINX_DISPLAYPORT_TX_ENHANCED_FRAME_EN, 0x00);
        lane_count = lane_count & 0x07;
    }

    status = xildpAUXWrite(XILINX_DISPLAYPORT_DPCD_LANE_COUNT_SET, 1, &lane_count);

    return status;
}


/*
 *  Function: xildpSetTrainingPattern
 *      Set the current training pattern for both source and sink device. Note that this function
 *      requires the use of the constants specified in displayport_defs.h.
 * 
 *	Parameters:
 *      pattern - : Select the current training pattern (1, 2 or 0 for off)
 *
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      
 *  (end)
 *      
*/
UINT32 xildpSetTrainingPattern(UINT32 pattern)
{
    UINT32 status = 0;
    UINT8 dpcd_pattern = XILINX_DISPLAYPORT_TRAINING_OFF;
    switch (pattern)
    {
        //case 0x01:
        case XILINX_DISPLAYPORT_TRAINING_PATTERN_1:
            dpcd_pattern = XILINX_DISPLAYPORT_TRAINING_PATTERN_1;
            break;
        //case 0x02:
        case XILINX_DISPLAYPORT_TRAINING_PATTERN_2:
            dpcd_pattern = XILINX_DISPLAYPORT_TRAINING_PATTERN_2;
            break;
        case XILINX_DISPLAYPORT_TRAINING_PATTERN_3:
            dpcd_pattern = XILINX_DISPLAYPORT_TRAINING_PATTERN_3;
            break;
        default:
            dpcd_pattern = XILINX_DISPLAYPORT_TRAINING_OFF;
            break;
    }
    // Write to the sink device
    status = xildpAUXWrite(XILINX_DISPLAYPORT_DPCD_TRAINING_PATTERN_SET, 1, &dpcd_pattern);
    // Write to source device
    dptx_reg_write(XILINX_DISPLAYPORT_TX_TRAINING_PATTERN_SET, (dpcd_pattern & 0x03));
    // Return the status of the write
    return status;
}

/*
 *  Function: xildpSetTrainingValues
 *      Writes current training values for voltage swing and preemphasis to both the source
 *      and sink devices.
 *  
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpSetTrainingValues();
 *  (end)
 *      
*/ 
UINT32 xildpSetTrainingValues(void)
{
    UINT32 status = 0;
    UINT8 data_buffer[4];

    //dbg3_printf("Setting TX PREEMP/VSWING to 0x%02x / 0x%02x \n\r", preset_preemp_tx, preset_vswing_tx);

    // Set voltage swing levels
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_0, preset_vswing_tx);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_1, preset_vswing_tx);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_2, preset_vswing_tx);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_3, preset_vswing_tx);

    // Set preemphasis to preset value
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_LANE_0, preset_preemp_tx);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_LANE_1, preset_preemp_tx);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_LANE_2, preset_preemp_tx);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_LANE_3, preset_preemp_tx);

    //7-series
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_0, preset_preemp_tx);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_1, preset_preemp_tx);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_2, preset_preemp_tx);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_3, preset_preemp_tx);

    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_0, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_1, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_2, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_3, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_0);

    data_buffer[0] = (preset_preemp_rx << 3) | preset_vswing_rx;
    data_buffer[1] = (preset_preemp_rx << 3) | preset_vswing_rx;
    data_buffer[2] = (preset_preemp_rx << 3) | preset_vswing_rx;
    data_buffer[3] = (preset_preemp_rx << 3) | preset_vswing_rx;

    // Write the values to the sink device
    //dbg3_printf("Setting RX PREEMP/VSWING to 0x%02x / 0x%02x \n\r", preset_preemp_rx, preset_vswing_rx);
    status = xildpAUXWrite(XILINX_DISPLAYPORT_DPCD_TRAINING_LANE0_SET, 4, data_buffer);

    return status;
}

/*
 *  Function: xildpSetMainLinkChannelCoding
 *      Set the value for the Main Link Channel Coding register, indicating support for 8b/10b
 *      encoding of the main link.
 *
 *  Parameters:
 *      enable_8b10b - : Enable / disable 8b/10b encoding (positive values enable, zero disables)
 *
 *  Returns:
 *      Zero on success, non-zero on failure
 *
 *  Example:
 *  (start code)
 *      UINT8 ml_coding = 0x01;
 *      UINT32 status = xildpSetMainLinkChannelCoding(&ml_coding);
 *  (end)
 *
*/
UINT32 xildpSetMainLinkChannelCoding(UINT8 enable_8b10b)
{
    UINT32 status = 0;

    // Constrain input value
    if (enable_8b10b > 0)
        enable_8b10b = 0x01;
    else
        enable_8b10b = 0x00;
    // Write the enable value to the sink device
    status = xildpAUXWrite(XILINX_DISPLAYPORT_DPCD_MAIN_LINK_CODING_SET, 1, &enable_8b10b);
    // Return the status of the AUX write
    return status;
}

/*
 *  Function: xildpSelectTrainingSettings
 *      Sets the preset values for link training. The training settings are a numerical value
 *      from 0 to 9 corresponding to each of the valid voltage swing / preemphasis level
 *      combinations allowed by the Displayport 1.1a specification. The table is indexed base
 *      on the voltage swing; the first four entries are all at voltage swing zero and interate
 *      over all valid preemphasis levels. Entries 5 to 8 are the valid preemphasis values for
 *      voltage swing level 1. This continues until maximum voltage swing is reached.
 *
 *	Parameters:
 *      training_settings - : the training set value to use
 * 
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpSelectTrainingSettings(5);
 *  (end)
 *      
*/ 
UINT32 xildpSelectTrainingSettings(UINT32 training_settings)
{
    UINT32 status = 0;

    switch(training_settings)
    {
        case 0: // VSWING 0
            xildpPresetVSwing(XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
            xildpPresetPreemphasis(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);
            break;

        case 1:
            xildpPresetVSwing(XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
            xildpPresetPreemphasis(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_1);
            break;

        case 2:
            xildpPresetVSwing(XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
            xildpPresetPreemphasis(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_2);
            break;

        case 3:
            xildpPresetVSwing(XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
            xildpPresetPreemphasis(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_3);
            break;

        case 4: // VSWING 1
            xildpPresetVSwing(XILINX_DISPLAYPORT_TX_PHY_VSWING_1);
            xildpPresetPreemphasis(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);
            break;

        case 5:
            xildpPresetVSwing(XILINX_DISPLAYPORT_TX_PHY_VSWING_1);
            xildpPresetPreemphasis(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_1);
            break;

        case 6:
            xildpPresetVSwing(XILINX_DISPLAYPORT_TX_PHY_VSWING_1);
            xildpPresetPreemphasis(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_2);
            break;

        case 7: // VSWING 2
            xildpPresetVSwing(XILINX_DISPLAYPORT_TX_PHY_VSWING_2);
            xildpPresetPreemphasis(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);
            break;

        case 8:
            xildpPresetVSwing(XILINX_DISPLAYPORT_TX_PHY_VSWING_2);
            xildpPresetPreemphasis(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_1);
            break;

        case 9: // VSWING 3
            xildpPresetVSwing(XILINX_DISPLAYPORT_TX_PHY_VSWING_3);
            xildpPresetPreemphasis(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);
            break;

        default:
            status = -1;
            break;
    }

    return status;
}


/*
 *  Function: xildpInitCoreTX
 *      Initialize the transmitter (source) to a known-good, default state.
 *  
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      None
 * 
 *	Example:
 *  (start code)
 *      xildpInitCoreTX();
 *  (end)
 *      
*/ 
void xildpInitCoreTX(void)
{
    // Disable the main and secondary streams
    dptx_reg_write(XILINX_DISPLAYPORT_TX_ENABLE_MAIN_STREAM, 0x00);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_ENABLE_SEC_STREAM, 0x00);
    // Clear any LQ pattern and enable scrambling
    dptx_reg_write(XILINX_DISPLAYPORT_TX_LINK_QUAL_PATTERN_SET, 0x00);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_SCRAMBLING_DISABLE, 0x00);
    // Enable enhanced framing mode and spread spectrum clocking
    dptx_reg_write(XILINX_DISPLAYPORT_TX_DOWNSPREAD_CTRL, 0x00);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_ENHANCED_FRAME_EN, 0x01);
    // Set up initial link configuration values
    dptx_reg_write(XILINX_DISPLAYPORT_TX_LINK_BW_SET, XILINX_DISPLAYPORT_LINK_RATE_2_7GBPS);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_LANE_COUNT_SET, 0x04);

    //7-series
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_0, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_1, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_2, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_LANE_3, XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0);

    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_0, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_1, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_2, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_LANE_3, XILINX_DISPLAYPORT_TX_PHY_PRECURSOR_0);
    // Set voltage swing to default levels
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_0, XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_1, XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_2, XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_3, XILINX_DISPLAYPORT_TX_PHY_VSWING_0);
}

/*
 *  Function: xildpVerifyTrainingConfiguration
 *      Validate the current transmitter setup; lane count, link speed and hpd assertion status are checked.
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      bool settings_valid = xildpVerifyTrainingConfiguration();
 *  (end)
 *      
*/ 
UINT32 xildpVerifyTrainingConfiguration(void)
{
    UINT32 status = XILINX_DISPLAYPORT_CONFIG_VALID;

    UINT32 link_rate = 0, lane_count = 0;

    link_rate = dptx_reg_read(XILINX_DISPLAYPORT_TX_LINK_BW_SET);
    lane_count = dptx_reg_read(XILINX_DISPLAYPORT_TX_LANE_COUNT_SET);

    // Validate link speed
    if (link_rate != 0x0A && link_rate != 0x06 && link_rate != 0x14)
    {
        status = XILINX_DISPLAYPORT_CONFIG_INVALID_LINK_RATE;
    }
    // Validate lane count
    if ((lane_count & 0x07) != 0x01 && (lane_count & 0x07) != 0x02 && (lane_count & 0x07) != 0x04)
    {
        status = XILINX_DISPLAYPORT_CONFIG_INVALID_LANE_COUNT;
    }
    // Verify a device is connected
    if (xildpGetHPDStatus() != 0x01)
    {
        status = XILINX_DISPLAYPORT_CONFIG_HPD_DEASSERTED;
    }

    return status;
}

/*
 *  Function: xildpSourceVSwingForValue
 *      Determine the correct voltage swing setting for the transmitter based on
 *      the specified DPCD voltage swing level. Valid range for the DPCD value
 *      is 0 to 3, other values will be ignored.
 *
 *	Parameters:
 *      dpcd_vswing - : DPCD voltage swing level to match
 *      
 *  Returns:
 *      Valid TX voltage swing constant, or 0xFF to indicate an invalid DPCD settings
 *                o  Voltage swing constants:
 *                o  XILINX_DISPLAYPORT_TX_PHY_VSWING_0
 *                o  XILINX_DISPLAYPORT_TX_PHY_VSWING_1
 *                o  XILINX_DISPLAYPORT_TX_PHY_VSWING_2
 *                o  XILINX_DISPLAYPORT_TX_PHY_VSWING_3
 * 
 *	Example:
 *  (start code)
 *      UINT32 tx_vswing = xildpSourceVSwingForValue(2);
 *  (end)
 *      
*/ 
UINT32 xildpSourceVSwingForValue(UINT32 dpcd_vswing)
{
    UINT32 tx_vswing = 0xFF;
    // Valid values are 400, 600, 800 and 1200 mvolts
    switch (dpcd_vswing)
    {
        case 0:
            tx_vswing = XILINX_DISPLAYPORT_TX_PHY_VSWING_0;
            break;
        case 1:
            tx_vswing = XILINX_DISPLAYPORT_TX_PHY_VSWING_1;
            break;
        case 2:
            tx_vswing = XILINX_DISPLAYPORT_TX_PHY_VSWING_2;
            break;
        case 3:
            tx_vswing = XILINX_DISPLAYPORT_TX_PHY_VSWING_3;
            break;
    }
    return tx_vswing;
}

/*
 *  Function: xildpSourcePreemphasisForValue
 *      Determine the correct preemphasis setting for the transmitter based on
 *      the specified DPCD preemphasis level. Valid range for the DPCD value
 *      is 0 to 3, other values will be ignored.
 *
 *	Parameters:
 *      dpcd_preemp    - : DPCD preemphasis level to match
 *      current_vswing - : current DPCD voltage swing level
 * 
 *  Returns:
 *      Valid TX preemphasis constant, or 0xFF to indicate an invalid DPCD settings
 *                o  Voltage swing constants:
 *                o  XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_0
 *                o  XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_1
 *                o  XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_2
 *                o  XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_3
 * 
 *	Example:
 *  (start code)
 *      UINT32 tx_preemp = xildpSourcePreemphasisForValue(3, 1);
 *  (end)
 *      
*/ 
UINT32 xildpSourcePreemphasisForValue(UINT32 dpcd_preemp, UINT32 current_vswing)
{
    // Current vswing limits max preemphasis, by Displayport spec
    UINT32 max_preemp = 0, tx_preemp = 0xFF;

    // TX preemphasis levels are the same for 0 and 1, so use Dport preemp + 1
    // to the get the correct range
    tx_preemp = dpcd_preemp + 1;

    switch (current_vswing)
    {
        case 0:
            max_preemp = 4;
            break;
        case 1:
            max_preemp = 3;
            break;
        case 2:
            max_preemp = 2;
            break;
        case 3:
            max_preemp = 1;
            break;
    }
    
    if (tx_preemp > max_preemp)
        tx_preemp = max_preemp;

    return tx_preemp;
}

/*
 *  Function: xildpDPCDVSwingForValue
 *      Determine the correct DPCD value for voltage swing based on the specified
 *      transmitter setting
 *
 *	Parameters:
 *      tx_vswing - : transmitter voltage swing to use
 * 
 *  Returns:
 *      DPCD value for voltage swing. Return value is 0 to 3 or 0xFF for failure:
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpDPCDVSwingForValue(XILINX_DISPLAYPORT_TX_PHY_VSWING_2);
 *  (end)
 *      
*/ 
UINT32 xildpDPCDVSwingForValue(UINT32 tx_vswing)
{
    UINT32 dpcd_value = 0xFF;
    // Valid values are 400, 600, 800 and 1200 mvolts
    switch (tx_vswing)
    {
        case XILINX_DISPLAYPORT_TX_PHY_VSWING_0:
            dpcd_value = 0;
            break;
        case XILINX_DISPLAYPORT_TX_PHY_VSWING_1:
            dpcd_value = 1;
            break;
        case XILINX_DISPLAYPORT_TX_PHY_VSWING_2:
            dpcd_value = 2;
            break;
        case XILINX_DISPLAYPORT_TX_PHY_VSWING_3:
            dpcd_value = 3;
            break;
    }

    dpcd_value = adj_req_lane01 & 0x3;

    // Set max level reached flag
    if (dpcd_value == 3)
        dpcd_value += 4;

    return dpcd_value;
}

/*
 *  Function: xildpDPCDPreemphasisForValue
 *      Determine the correct DPCD value for preemphasis based on the specified
 *      transmitter setting
 *
 *	Parameters:
 *      tx_vswing      - : transmitter preemphasis to use
 *      current_vswing - : current voltage swing setting for the transmitter
 * 
 *  Returns:
 *      DPCD value for preemphasis. Return value is 0 to 3 or 0xFF for failure:
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpDPCDPreemphasisForValue(XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_1, XILINX_DISPLAYPORT_TX_PHY_VSWING_2);
 *  (end)
 *      
*/ 
UINT32 xildpDPCDPreemphasisForValue(UINT32 tx_preemp, UINT32 current_vswing)
{
    // Current vswing limits max preemphasis, by Displayport spec
    UINT32 max_preemp = 0, dpcd_value = 0;

    dpcd_value = tx_preemp - 1;

    switch (current_vswing)
    {
        case XILINX_DISPLAYPORT_TX_PHY_VSWING_0:
            max_preemp = 3;
            break;
        case XILINX_DISPLAYPORT_TX_PHY_VSWING_1:
            max_preemp = 2;
            break;
        case XILINX_DISPLAYPORT_TX_PHY_VSWING_2:
            max_preemp = 1;
            break;
        case XILINX_DISPLAYPORT_TX_PHY_VSWING_3:
            max_preemp = 0;
            break;
    }
    // Check for max value
    if (dpcd_value > max_preemp)
        dpcd_value = max_preemp;


    dpcd_value = ( adj_req_lane01 >> 2 ) & 0x3;

    // Set max level reached flag
    if (dpcd_value == 3)
        dpcd_value += 0x04;

    //dbg3_printf("Updated RX Pre-emphasis is %x\n\r",dpcd_value);
    return dpcd_value;
}

/*
 *  Function: xildpPresetVSwing
 *      Sets the preset voltage swing to the specified level. Typically this function is not called
 *      directly, as it is used by the <xildpSelectTrainingSettings> function.
 *
 *	Parameters:
 *      fixed_vswing - : transmitter voltage swing level to set
 * 
 *  Returns:
 *      None
 * 
 *	Example:
 *  (start code)
 *      xildpPresetVSwing(1);
 *  (end)
 *      
*/ 
void xildpPresetVSwing(UINT32 fixed_vswing)
{
    preset_vswing_tx = fixed_vswing;
    preset_vswing_rx = xildpDPCDVSwingForValue(preset_vswing_tx);
    //dbg3_printf("Debug preset_vswing_tx %x, preset_vswing_rx %x\n\r",preset_vswing_tx,preset_vswing_rx);
}

/*
 *  Function: xildpPresetPreemphasis
 *      Sets the preset preemphasis to the specified level. Typically this function is not called
 *      directly, as it is used by the <xildpSelectTrainingSettings> function.
 *
 *	Parameters:
 *      fixed_preemp - : transmitter preemphasis level to set
 * 
 *  Returns:
 *      None
 * 
 *	Example:
 *  (start code)
 *      xildpPresetPreemphasis(2);
 *  (end)
 *      
*/ 
void xildpPresetPreemphasis(UINT32 fixed_preemp)
{
    preset_preemp_tx = fixed_preemp;
    preset_preemp_rx = xildpDPCDPreemphasisForValue(preset_preemp_tx, preset_vswing_tx);
}

/*
 *  Function: xildpRunClockRecovery
 *      Runs a single iteration of the clock recovery loop (training pattern 1)
 *      Does NOT change settings, does NOT verify configuration
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpRunClockRecovery();
 *  (end)
 *      
*/ 
UINT32 xildpRunClockRecovery(void)
{
    UINT32 status = 0, lane_count = 0;
    UINT8 cr_done[4], aux_read_data[6];
    UINT8 clock_recovery_done = 0;

    // Get the lane count
    lane_count = dptx_reg_read(XILINX_DISPLAYPORT_TX_LANE_COUNT_SET);
    // Wait time for clock recovery
    MB_Sleep(1);
    //xildpWaituS(XILINX_DISPLAYPORT_CLOCK_REC_TIMEOUT);
    // Read DPCD status bits
    status = xildpAUXRead(XILINX_DISPLAYPORT_DPCD_STATUS_LANE_0_1, 6, aux_read_data);
    if (status)
    {
        // Clear cr_done flags on failure of AUX transaction
        cr_done[0] = 0;
        cr_done[1] = 0;
        cr_done[2] = 0;
        cr_done[3] = 0;
    }
    else
    {    
        adj_req_lane01 = aux_read_data[4];
        adj_req_lane23 = aux_read_data[5];
    
        // Clock recovery bits
        cr_done[0] = (aux_read_data[0] & XILINX_DISPLAYPORT_LANE_0_STATUS_CLK_REC_DONE);
        cr_done[1] = (aux_read_data[0] & XILINX_DISPLAYPORT_LANE_1_STATUS_CLK_REC_DONE) >> 4;
        cr_done[2] = (aux_read_data[1] & XILINX_DISPLAYPORT_LANE_2_STATUS_CLK_REC_DONE);
        cr_done[3] = (aux_read_data[1] & XILINX_DISPLAYPORT_LANE_3_STATUS_CLK_REC_DONE) >> 4;
    
        // Sum up clock recoverry done bits
        clock_recovery_done = cr_done[0] + cr_done[1] + cr_done[2] + cr_done[3];
    }
        // Verify status is equal to lane count
        if (clock_recovery_done == lane_count )
            status = XILINX_DISPLAYPORT_OPERATION_SUCCESS;
        else
            status = XILINX_DISPLAYPORT_OPERATION_FAILED;
    return status;
}

/*
 *  Function: xildpRunChannelEqualization
 *      Run the channel equalization routine (training pattern 2) to achieve channel equalization,
 *      symbol lock and interlane alignment.
 *  
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpRunChannelEqualization();
 *  (end)
 *      
*/ 
UINT32 xildpRunChannelEqualization(void)
{
    UINT32 lane_count = 0, status = 0, cr_valid = 0;
    UINT32 channel_equalization_done = 0, symbol_lock_done = 0, lane_align_done = 0;
    UINT8 chan_eq_done[4], sym_lock_done[4];
    UINT8 aux_read_data[6];

    // Get the lane count
    lane_count = dptx_reg_read(XILINX_DISPLAYPORT_TX_LANE_COUNT_SET);
    // Wait time for channel equalization
    MB_Sleep(1);
    //xildpWaituS(XILINX_DISPLAYPORT_CHAN_EQ_TIMEOUT);
    // Read DPCD status bits
    status = xildpAUXRead(XILINX_DISPLAYPORT_DPCD_STATUS_LANE_0_1, 6, aux_read_data);
    if (status)
    {
        // Clear flags
        memset(chan_eq_done, 0, 4);
        memset(sym_lock_done, 0, 4);
        // Failed read 
        status = XILINX_DISPLAYPORT_OPERATION_FAILED;
    }
    else
    {
        adj_req_lane01 = aux_read_data[4];
        adj_req_lane23 = aux_read_data[5];
    
        // Verify CR is still valid
        cr_valid = ( aux_read_data[0] & XILINX_DISPLAYPORT_LANE_0_STATUS_CLK_REC_DONE) + 
                   ((aux_read_data[0] & XILINX_DISPLAYPORT_LANE_1_STATUS_CLK_REC_DONE) >> 4)  + 
                   ( aux_read_data[1] & XILINX_DISPLAYPORT_LANE_2_STATUS_CLK_REC_DONE) + 
                   ((aux_read_data[1] & XILINX_DISPLAYPORT_LANE_3_STATUS_CLK_REC_DONE) >> 4);
    
        if (cr_valid != lane_count)
        {
            status = XILINX_DISPLAYPORT_CR_FAILED;
        }
        else
        {
            // Channel Equalization bits
            chan_eq_done[0]  = (aux_read_data[0] & XILINX_DISPLAYPORT_LANE_0_STATUS_CHAN_EQ_DONE) >> 1; 
            chan_eq_done[1]  = (aux_read_data[0] & XILINX_DISPLAYPORT_LANE_1_STATUS_CHAN_EQ_DONE) >> 5; 
            chan_eq_done[2]  = (aux_read_data[1] & XILINX_DISPLAYPORT_LANE_2_STATUS_CHAN_EQ_DONE) >> 1; 
            chan_eq_done[3]  = (aux_read_data[1] & XILINX_DISPLAYPORT_LANE_3_STATUS_CHAN_EQ_DONE) >> 5; 
        
            // Symbol Lock bits
            sym_lock_done[0] = (aux_read_data[0] & XILINX_DISPLAYPORT_LANE_0_STATUS_SYM_LOCK_DONE) >> 2;
            sym_lock_done[1] = (aux_read_data[0] & XILINX_DISPLAYPORT_LANE_1_STATUS_SYM_LOCK_DONE) >> 6;
            sym_lock_done[2] = (aux_read_data[1] & XILINX_DISPLAYPORT_LANE_2_STATUS_SYM_LOCK_DONE) >> 2;
            sym_lock_done[3] = (aux_read_data[1] & XILINX_DISPLAYPORT_LANE_3_STATUS_SYM_LOCK_DONE) >> 6;
        
            // Sum up channel qualization and symbol lock 'done' bits
            channel_equalization_done = chan_eq_done[0] + chan_eq_done[1] + chan_eq_done[2] + chan_eq_done[3];
            symbol_lock_done = sym_lock_done[0] + sym_lock_done[1] + sym_lock_done[2] + sym_lock_done[3];
        
            // Each time through, check lane alignment bit
            lane_align_done = (aux_read_data[2] & XILINX_DISPLAYPORT_LANE_ALIGNMENT_DONE);
            // Verify all lanes are valid
            if (channel_equalization_done == lane_count && symbol_lock_done == lane_count &&
                lane_align_done == XILINX_DISPLAYPORT_LANE_ALIGNMENT_DONE)
            {
                status = XILINX_DISPLAYPORT_OPERATION_SUCCESS;
            }
            else
                status = XILINX_DISPLAYPORT_OPERATION_FAILED;
        }
    }
    return status;
}

/*
 *  Function: xildpRunClockRecoveryLoop
 *      Run the clock recovery loop for the specified number of iterations. This function calls
 *      the xildpRunClockRecovery() function.
 *  
 *	Parameters:
 *      max_iterations - : Number of times to execute the loop
 * 
 *  Returns:
 *      Status of the last executed clock recovery iteration
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpRunClockRecoveryLoop(MAX_TRAINING_ITERATIONS, 1);
 *  (end)
 *      
*/ 
UINT32 xildpRunClockRecoveryLoop(UINT32 max_iterations, UINT32 adaptive)
{
    UINT32 status = 0, xx = 0, done = 0;


    // start from 0 volatge and pre-emphasis levels..
    adj_req_lane01 = 0;
    adj_req_lane23 = 0;
    preset_vswing_tx = XILINX_DISPLAYPORT_TX_PHY_VSWING_0;
    preset_preemp_tx = XILINX_DISPLAYPORT_TX_PHY_POSTCURSOR_0;

    preset_vswing_rx = 0;
    preset_preemp_rx = 0;
    // clear voltage/pre-emp levels..
    xildpSetTrainingValues();
    // Set the training pattern
    xildpSetTrainingPattern(XILINX_DISPLAYPORT_TRAINING_PATTERN_1);
    //xildpSetTrainingValues();

    while (!done)
    {
        // Set training values
        xildpSetTrainingValues();
        // Run clock recovery once
        status = xildpRunClockRecovery();
        // dbg_llc_printf ("CDR loop :: adj_req_lane01 %8x, adj_req_lane23 %8x\n\r",adj_req_lane01,adj_req_lane23);
        switch (status)
        {
            case XILINX_DISPLAYPORT_OPERATION_SUCCESS:
            	done = 1;
                break;
            default:
                if (adaptive == 1) // Adaptive training run (read adj req)
                {
                    // Check for max vswing,
                	// exit if the max training has been run atleast once.
                    if ((preset_vswing_rx & 0x07) == 0x07){
                        done = 1;
                        //dbg1_llc_printf("Exiting CDR loop as max voltage levels reached...");
                    }

                    xildpRunTrainingAdjustment();

                }
                break;
        }
        // updated read training values..
        // xildpSetTrainingValues();
        // Increment loop counter
        if (++xx == max_iterations)
            done = 1;
    }

    // Return status code
    return status;
}

/*
 *  Function: xildpRunChannelEqualizationLoop
 *      Run the channel equalization loop for the specified number of iterations. This function calls
 *      the xildpRunChannelEqualization() function.
 *   
 *	Parameters:
 *      max_iterations - : Number of times to execute the loop
 * 
 *  Returns:
 *      Status of the last executed channel equalization iteration
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpRunChannelEqualizationLoop(MAX_TRAINING_ITERATIONS, 1);
 *  (end)
 *      
*/ 
UINT32 xildpRunChannelEqualizationLoop(UINT32 max_iterations, UINT32 adaptive, UINT8 link_rate)
{
    UINT32 status = 0, xx = 0, done = 0;

    // Set the training pattern
    //if (adaptive == 1) // Adaptive training run (read adj req)
        //xildpRunTrainingAdjustment();

    //xildpSetTrainingValues();
    if(link_rate == XILINX_DISPLAYPORT_LINK_RATE_5_4GBPS)
      xildpSetTrainingPattern(XILINX_DISPLAYPORT_TRAINING_PATTERN_2);
    else
      xildpSetTrainingPattern(XILINX_DISPLAYPORT_TRAINING_PATTERN_2);
    
    // CE/SL training loop
    while (!done)
    {
        // Set training values
        xildpSetTrainingValues();
        // Run clock recovery once
        status = xildpRunChannelEqualization();
        switch (status)
        {
            case XILINX_DISPLAYPORT_CR_FAILED:
                done = 1;
                break;
            case XILINX_DISPLAYPORT_OPERATION_SUCCESS:
            	/*
            	status = xildpRunChannelEqualization();
            	if ( status != XILINX_DISPLAYPORT_OPERATION_SUCCESS ) {
            		done = 0; // continue to train EQ again again.
            	} else {
                    done = 1;
            	}
            	*/
            	done = 1;
                break;
            default:
                if (adaptive == 1) // Adaptive training run (read adj req)
                    xildpRunTrainingAdjustment(); // ARUN
                //xildpSetTrainingValues();
                break;
        }
        // Increment loop counter
        if ( (++xx == max_iterations) && ( done == 0 ) )
        {
        	xildpSetTrainingValues();
            done = 1;
        }
    }
    // Return status code
    return status;
}

/*
 *  Function: xildpRunTrainingAdjustment
 *      Process an adjustment request from the sink device during training.
 *  
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpRunTrainingAdjustment();
 *  (end)
 *      
*/ 
UINT32 xildpRunTrainingAdjustment(void)
{
    UINT32 status = 0, training_set = 0;
    UINT8 vs_level[4], pre_level[4];

    vs_level[0]  =  adj_req_lane01 & 0x03;
    pre_level[0] = (adj_req_lane01 >> 2) & 0x03;

    //dbg3_printf("DEBUG ---> adj_req_lane01 %x %x %x \n\r",adj_req_lane01,vs_level[0],pre_level[0]);
    // See what the base value for the training settings should be
    switch (vs_level[0])
    {
        case 0:
            training_set = 0;
            break;
        case 1:
            training_set = 4;
            break;
        case 2:
            training_set = 7;
            break;
        case 3:
            training_set = 9;
            break;
    }
    // Increment the set by the requests preemphasis level
    training_set += pre_level[0];
    // Select the new settings
    status = xildpSelectTrainingSettings(training_set);

    return status;
}

/*
 *  Function: xildpRunAdaptiveTrainingLoop
 *      Run a complete training loop using the specified settings as a base. This routine will adjust voltage
 *      swing and preemphasis levels based on feedback from the sink device in the Adjustment Request registers
 *      in the DPCD.
 *      The setting for preserve_linkrate is used during loop-through testing to enable the receiver to send
 *      video data to the transmitter. This parameter should be zero for most applications.
 *  
 *	Parameters:
 *      training_settings   - : One of the 10 valid settings to use for voltage swing and preemphasis levels
 *      adaptive            - : Adjust training to requests from the sink device
 *      preserve_linkrate   - : Synchronize link rate between integrated TX / RX devices
 * 
 *  Returns:
 *      Combined status value for all active lines, interlane alignment and lane status updated.
 *                o Byte configuration is as follows:
 *                o alignment << 16 | status_lanes_23 << 8 | status_lanes_01
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpRunTrainingLoop(0, 1, 0)
 *  (end)
 *      
*/ 
UINT32 xildpRunTrainingLoop(UINT32 training_settings, UINT32 adaptive, UINT32 preserve_linkrate)
{
    UINT32 status = 0, lanes = 0;
    UINT8  aux_read_data[4];
    UINT32 done = 0;
    UINT32 training_state = XILINX_DISPLAYPORT_TS_CLOCK_REC;
    XILDPMainLinkConfig *mlconf = dplpmGetMainLinkConfig();

    // Clear buffers
    memset(aux_read_data, 0, 4);

    // Validate configuration
    if ( xildpVerifyTrainingConfiguration() )
        return XILINX_DISPLAYPORT_OPERATION_FAILED;

    // Reset adjustment request data
    adj_req_lane01 = adj_req_lane23 = 0;
    // Set the base training values
    xildpSelectTrainingSettings(training_settings);

    // Verify adaptive parameter
    if (adaptive > 0)
        adaptive = 1;

    // Turn off scrambling for training
    dptx_reg_write(XILINX_DISPLAYPORT_TX_SCRAMBLING_DISABLE, 0x01);

    // Enter training loop
    while( !done )
    {
        switch (training_state)
        {
            case XILINX_DISPLAYPORT_TS_CLOCK_REC:
                // Clock recovery loop
                status = xildpRunClockRecoveryLoop(XILINX_DISPLAYPORT_MAX_TRAINING_ATTEMPTS, adaptive);
                if (status && preserve_linkrate == 0)
                {
                    dbg_printf("CR: ");
                    status = xildpUpdateStatus();
                    training_state = XILINX_DISPLAYPORT_TS_ADJUST_SPD;
                }
                else
                    training_state = XILINX_DISPLAYPORT_TS_CHANNEL_EQ;
                break;
            case XILINX_DISPLAYPORT_TS_CHANNEL_EQ:
                status = xildpRunChannelEqualizationLoop(XILINX_DISPLAYPORT_MAX_TRAINING_ATTEMPTS, adaptive, dptx_reg_read(XILINX_DISPLAYPORT_TX_LINK_BW_SET));
                if (status && preserve_linkrate == 0)
                {
                    dbg_printf("CE: ");
                    status = xildpUpdateStatus();
                    training_state = XILINX_DISPLAYPORT_TS_ADJUST_SPD;
                }
                else
                    done = 1;
                break;
            case XILINX_DISPLAYPORT_TS_ADJUST_SPD:
                // If we're at high speed, downshift and try again
                if (dptx_reg_read(XILINX_DISPLAYPORT_TX_LINK_BW_SET) == XILINX_DISPLAYPORT_LINK_RATE_5_4GBPS)
                {
                    dbg_printf("Downshifting to 2.7 Gbps\n\r");
                    lanes = dptx_reg_read(XILINX_DISPLAYPORT_TX_LANE_COUNT_SET);
                    mlconf->link_rate = 0xA;
                    xildpSetLinkRate(XILINX_DISPLAYPORT_LINK_RATE_2_7GBPS);
                    xildpSetLaneCount(lanes);
                    // Back to clock recovery to restart training
                    training_state = XILINX_DISPLAYPORT_TS_CLOCK_REC;
                }
                else
                if (dptx_reg_read(XILINX_DISPLAYPORT_TX_LINK_BW_SET) == XILINX_DISPLAYPORT_LINK_RATE_2_7GBPS)
                {
                    dbg_printf("Downshifting to 1.62 Gbps\n\r");
                    lanes = dptx_reg_read(XILINX_DISPLAYPORT_TX_LANE_COUNT_SET);
                    mlconf->link_rate = 0x6;
                    xildpSetLinkRate(XILINX_DISPLAYPORT_LINK_RATE_1_62GBPS);
                    xildpSetLaneCount(lanes);
                    // Back to clock recovery to restart training
                    training_state = XILINX_DISPLAYPORT_TS_CLOCK_REC;
                }
                else
                {
                    dbg_printf("ADJ: ");
                    //status = xildpUpdateStatus();
                    done = 1;
                }
                break;
        }
    }

    // Turn off training pattern
    xildpSetTrainingPattern(XILINX_DISPLAYPORT_TRAINING_OFF);
    // Turn on scrambling after training
    dptx_reg_write(XILINX_DISPLAYPORT_TX_SCRAMBLING_DISABLE, 0x00);
    // Do a final status read here 
    status = xildpUpdateStatus();
    // Return complete status dword
    return status;
}

/*
 *  Function: xildpUpdateStatus
 *      Retrieves the current link status from the attached sink device. 
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      status - : 32-bit value containing current link status including interlane alignment bit
 * 
 *	Example:
 *  (start code)
 *      UINT32 link_status = xildpUpdateStatus();
 *  (end)
 *      
*/ 
UINT32 xildpUpdateStatus(void)
{
    UINT32 status = 0;
    UINT8 aux_read_data[3];

    status = xildpAUXRead(XILINX_DISPLAYPORT_DPCD_STATUS_LANE_0_1, 3, aux_read_data);
    if (!status)
        status = aux_read_data[2] << 16 | aux_read_data[1] << 8 | aux_read_data[0];
    return status;
}

/*
 *  Function: xildpGetDPCDRegName
 *      Retrieves the mnemonic for the specified DPCD register address 
 *
 *	Parameters:
 *      address - : the register address to look up
 * 
 *  Returns:
 *      A null-terminated C-string containing the register name
 * 
 *	Example:
 *  (start code)
 *      char * reg_name = xildpGetDPCDRegName(0x202);
 *  (end)
 *      
*/ 
char *xildpGetDPCDRegName(UINT32 address)
{
    // Register name array
    static char *dpcd_reg_names[] = 
    {
        "DPCD Revision               ", // 0 (0x00)
        "Max Link Rate               ", // 1
        "Max Lane Count              ", // 2
        "Max Downspread              ", // 3
        "Num Receiver Ports          ", // 4
        "Downstream Ports Present    ", // 5
        "Main Link Channel Coding    ", // 6
        "Downstream Port Info        ", // 7
        "Receiver Port 0 Caps 0      ", // 8
        "Receiver Port 0 Caps 1      ", // 9
        "Receiver Port 1 Caps 0      ", // 10
        "Receiver Port 1 Caps 1      ", // 11
        "Downstream Port Caps        ", // 12 0x80-0x8F
        "Link Bandwidth Setup        ", // 13 0x100
        "Lane Count Set              ", // 14
        "Training Patern Set         ", // 15
        "Training Lane 0 Set         ", // 16
        "Training Lane 1 Set         ", // 17
        "Training Lane 2 Set         ", // 18
        "Training Lane 3 Set         ", // 19
        "Downspread Ctrl             ", // 20
        "Main Link Channel Coding Set", // 21
        "Sink Count                  ", // 22 0x200
        "Device Service IRQ Vector   ", // 23
        "Lane Status 0/1             ", // 24
        "Lane Status 2/3             ", // 25
        "Lane Align Status Updated   ", // 26
        "Sink Status                 ", // 27
        "Adjustment Request Lanes 0-1", // 28
        "Adjustment Request Lanes 2-3", // 29
        "Training Score Lane 0       ", // 30
        "Training Score Lane 1       ", // 31
        "Training Score Lane 2       ", // 32
        "Training Score Lane 3       ", // 33
        "Symbol Error Count Lane 0l  ", // 34 0x210
        "Symbol Error Count Lane 0h  ", // 35
        "Symbol Error Count Lane 1l  ", // 36
        "Symbol Error Count Lane 1h  ", // 37
        "Symbol Error Count Lane 2l  ", // 38
        "Symbol Error Count Lane 2h  ", // 39
        "Symbol Error Count Lane 3l  ", // 40
        "Symbol Error Count Lane 3h  ", // 41
        "Test Request                ", // 42
        "Test Link Rate              ", // 43
        "Test Lane Count             ", // 44
        "Test Pattern                ", // 45
        "Power State Set             ", // 46
        "Unknown Register Name       "  // 47
    };

    char *reg_name = NULL;
    // First 12 registers
    if (address <= 0x0B)
        reg_name = dpcd_reg_names[address];
    // Downstream port caps
    if (address >= 0x80 && address <= 0x8F)
        reg_name = dpcd_reg_names[12];
    // Config registers
    if (address >= 0x100 && address <= 0x108)
        reg_name = dpcd_reg_names[13 + (address - 0x100)];
    // Status
    if (address >= 0x200 && address <= 0x20B)
        reg_name = dpcd_reg_names[22 + (address - 0x200)];
    // These are 16-bit values  for symbol error count
    if (address >= 0x210 && address <= 0x219)
        reg_name = dpcd_reg_names[34 + (address - 0x210)];
    // Test registers
    if (address >= 0x220 && address <= 0x221)
        reg_name = dpcd_reg_names[44 + (address - 0x220)];
    if (address == 0x600)
        reg_name = dpcd_reg_names[46];
    // Catch the failure case
    if (reg_name == NULL)
        reg_name = dpcd_reg_names[47];

    return reg_name;
}

/*
 *  Function: xildpAUXSubmitCommand
 *      Submits an AUX command for processing. This function will wait for a reply and return the
 *      response code.
 *
 *  Parameters:
 *      request - : Pointer to a valid and initialized XILDPAUXTransaction structure
 *
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpAUXSubmitCommand(&tx_req);
 *  (end)
 *      
 */
UINT32 xildpAUXSubmitCommand(XILDPAUXTransaction *request)
{
    UINT32 reply_status  = XILINX_DISPLAYPORT_OPERATION_TIMEOUT;
    volatile UINT32 reply_code    = 0;
    UINT32 defer_count   = 0; // Max = 7
    UINT32 timeout_count = 0; // 5 * 400us = 2ms total timeout

#if 0
    // Read these registers continually
    volatile UINT32 hpd      = 0;
    volatile UINT32 hpd_intr = 0;
    static UINT32 prev_hpd = 0;
    static UINT32 prev_intr = 0;
    XILDPMainLinkConfig *mlconf = dplpmGetMainLinkConfig();
    UINT32 loop_break = 0;
    mlconf->hpd_edge_seen = 0;
 #endif


    //dbg3_printf("xildpAUXSubmitCommand\n\r");

    while(reply_status == XILINX_DISPLAYPORT_OPERATION_TIMEOUT && 
          defer_count   < XILINX_DISPLAYPORT_MAX_DEFER_COUNT   && 
          timeout_count < XILINX_DISPLAYPORT_MAX_TIMEOUT_COUNT )
    {
        dptx_send_request(request);
        reply_status = dptx_wait_reply();
        if (reply_status != XILINX_DISPLAYPORT_OPERATION_TIMEOUT)
        {
            //dbg4_printf(".");
            reply_code = dptx_reg_read(XILINX_DISPLAYPORT_TX_RCV_REPLY_REG);
            switch (reply_code)
            {
                case XILINX_DISPLAYPORT_CMD_REPLY_I2C_ACK:   // Success value is 0x00 for both AUX and I2C
                    //dbg_printf("I2C ACK\n\r");
                    break;
                case XILINX_DISPLAYPORT_CMD_REPLY_I2C_DEFER: // I2C transaction deferred, try again
                	//dbg_llc_printf("I2C Defer\n\r");
                	MB_Sleep(1);
                	//xildpWaituS(100);

                	 if(request->exit_when_defer==1)
                	 {
                         reply_code = XILINX_DISPLAYPORT_OPERATION_FAILED;
                         defer_count = XILINX_DISPLAYPORT_MAX_DEFER_COUNT;
                	 }
                	 else
                	 {
                         reply_status = XILINX_DISPLAYPORT_OPERATION_TIMEOUT;
                         defer_count++;
                	 }
                    break;
                case XILINX_DISPLAYPORT_CMD_REPLY_I2C_NACK:  // I2C NACK
                    dbg_printf("AUX I2C Read Failed (NACK)\n\r");
                    reply_code = XILINX_DISPLAYPORT_OPERATION_FAILED;
                    break;
                case XILINX_DISPLAYPORT_CMD_REPLY_DEFER:     // AUX transaction deferred
                    //dbg_printf("AUX Defer\n\r");
                	MB_Sleep(1);
                	//xildpWaituS(100);
                    reply_status = XILINX_DISPLAYPORT_OPERATION_TIMEOUT;
                    defer_count++;
                    break;
                case XILINX_DISPLAYPORT_CMD_REPLY_NACK:
                    dbg_printf("AUX Transaction Failed (NACK)\n\r");
                    reply_code = XILINX_DISPLAYPORT_OPERATION_FAILED;
                    break;
                case XILINX_DISPLAYPORT_CMD_REPLY_INVALID:
                    dbg_printf("AUX Transaction Failed (Invalid Reply)\n\r");
                    // Received an invalid AUX transaction
                    reply_status = XILINX_DISPLAYPORT_INVALID_REPLY_CODE;
                    break;
                default: // Unknown or illegal response code
                    dbg_printf("Reply code 0x%02x unknown. AUX I2C Read Failed\n\r", reply_code);
                    reply_code = XILINX_DISPLAYPORT_OPERATION_FAILED;
                    break;
            }
        }
        else // Timeout failsafe to avoid an infinite loop
        {
        	MB_Sleep(1);
        	//xildpWaituS(100);
            timeout_count++;
        }
#if 0
        hpd      = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT_SIG_STATE) & XILINX_DISPLAYPORT_TX_AUX_HPD;
        hpd_intr = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT);
        if ( ( ( prev_intr & 0x3 ) == ( hpd_intr & 0x3 ) ) && ( prev_hpd == 0 ) &&  ( hpd == 1 ) ) {
           	dbg_printf("HPD Rising Edge seen in the middle of AUX - and No IRQ hpd %x intr %x prev hpd %x intr %x\r\n", hpd, hpd_intr, prev_hpd, prev_intr );
           	mlconf->hpd_edge_seen = 1;

        }
        prev_hpd = hpd;
        prev_intr = hpd_intr;
       //	if ( ( ( defer_count > 50 ) || ( timeout_count > 50 ) ) && ( mlconf->hpd_edge_seen == 1 ) ){
    	if ( mlconf->hpd_edge_seen == 1  ){
       		loop_break = 1;
       	}
#endif
    }

    if ( defer_count   >= XILINX_DISPLAYPORT_MAX_DEFER_COUNT   ||
         timeout_count >= XILINX_DISPLAYPORT_MAX_TIMEOUT_COUNT ) {
    	dbg_printf("\n\rAborted AUX RD: defer count:   %d   timeout count: %d \n\r",defer_count, timeout_count);
    }
#if 0
    else if ( loop_break ){
    	dbg_printf("\n\rAborted AUX RD: as HPD Edge & defer count:  %d   timeout count: %d \n\r",defer_count, timeout_count);
    }
#endif

    // Return the final reply code
    return reply_code;
}

/*
 *  Function: xildpI2CWrite
 *
 *  Performs an I2C write using the AUX channel commanded transaction mode.
 *
 *  Parameters:
 *     device_address   - : Seven bit I2C device address
 *     register_address - : Device subaddress to access
 *     byte_count       - : number of bytes to write to the sink (max is 16)
 *     data_buffer      - : pointer to a buffer containing data to write
 *
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpI2CWrite(0xA0, 0x00, 8, (UINT8*)d_buf);
 *  (end)
 *      
 */
UINT32 xildpI2CWrite(UINT32 device_address, UINT32 register_address, UINT32 byte_count, UINT8* data_buffer)
{
    UINT32 status = 0;
    UINT8 aux_data[4];

    Xboolean transaction_complete = FALSE;
    XILDPAUXTransaction request;

    // Make sure there's a sink device out there
    if (xildpGetHPDStatus() != 0x01)
        return XILINX_DISPLAYPORT_NOT_CONNECTED;

    // Cap the byte count
    if (byte_count > 16)
        byte_count = 16;

    aux_data[0] = register_address;
    aux_data[1] = 0x00;

    // Setup command
    request.cmd_code  = XILINX_DISPLAYPORT_CMD_REQUEST_I2C_WRITE_MOT;
    request.address   = device_address;
    request.num_bytes = 1;
    request.wr_data = aux_data;

    // Send the address write
    status = xildpAUXSubmitCommand(&request);
    if (status != XILINX_DISPLAYPORT_CMD_REPLY_ACK)
    {
        dbg_printf("Error: Writing I2C device address failed with code 0x%02x\n\r", status);
        return XILINX_DISPLAYPORT_OPERATION_FAILED;
    }

    // Submit the write command to hardware
    request.cmd_code  = XILINX_DISPLAYPORT_CMD_REQUEST_I2C_WRITE;
    request.address   = device_address;
    request.num_bytes = byte_count;
    request.wr_data   = data_buffer;

    while (!transaction_complete)
    {
        status = xildpAUXSubmitCommand(&request);
        if (status != XILINX_DISPLAYPORT_CMD_REPLY_ACK)
        {
            dbg_printf("Error: Writing I2C data failed with code 0x%02x\n\r", status);
            transaction_complete = TRUE;
            status = XILINX_DISPLAYPORT_OPERATION_FAILED;
        }
    }
    MB_Sleep(1);
    //xildpWaituS(100);

    return status;
}

/*
 *  Function: xildpI2CRead
 *
 *  Performs an I2C read using the AUX channel commanded transaction mode.
 *
 *  Parameters:
 *     device_address   - : Seven bit I2C device address
 *     register_address - : Device subaddress to access
 *     byte_count       - : number of bytes to read from the sink (max is 16)
 *     data_buffer      - : pointer to a buffer to place the resulting read data
 *
 *  Returns:
 *      Zero on success, non-zero on failure
 * 
 *	Example:
 *  (start code)
 *      UINT32 status = xildpI2CWrite(0xA0, 0x00, 8, (UINT8*)d_buf);
 *  (end)
 *      
 */
UINT32 xildpI2CRead(UINT32 device_address, UINT32 register_address, UINT32 byte_count, UINT8* data_buffer, UINT8 first_req, UINT8 last_req, UINT8 exit_when_defer)
{
    UINT32 status = 0;
    UINT32 data_count;
    UINT32 data_index;
    UINT32 reply_code;
    UINT32 xx;
    UINT8 aux_data[4];
    UINT8 done = 0;

    XILDPAUXTransaction request;

    // Make sure there's a sink device out there
    if (xildpGetHPDStatus() != 0x01)
        return XILINX_DISPLAYPORT_NOT_CONNECTED;

    // Cap the byte count
    if (byte_count > 16)
        byte_count = 16;

    aux_data[0] = register_address;
    aux_data[1] = 0x00;

    request.exit_when_defer = exit_when_defer;

    if ( first_req ){
        // Setup command
        request.cmd_code  = XILINX_DISPLAYPORT_CMD_REQUEST_I2C_WRITE_MOT;
        request.address   = device_address;
        request.num_bytes = 2;
        request.wr_data = aux_data;

        // Send the address write
        status = xildpAUXSubmitCommand(&request);
        if (status != XILINX_DISPLAYPORT_CMD_REPLY_ACK)
        {
            dbg_printf("Error: Writing I2C device address failed with code 0x%02x\n\r", status);
            return XILINX_DISPLAYPORT_OPERATION_FAILED;
        }
    }

    if ( last_req ) { // only - set MOT to 0
    	request.cmd_code  = XILINX_DISPLAYPORT_CMD_REQUEST_I2C_READ;
    } else { // MOT to 1
    	request.cmd_code  = XILINX_DISPLAYPORT_CMD_REQUEST_I2C_READ_MOT;
    }
    // Submit the read command to hardware
    request.address   = device_address;
    request.num_bytes = byte_count;

    data_count = 0;
    data_index = 0;

    while (data_count != byte_count && !done)
    {
        status = xildpAUXSubmitCommand(&request);
        if (status == XILINX_DISPLAYPORT_CMD_REPLY_ACK)
        {
            // Check for read data
            reply_code = dptx_reg_read(XILINX_DISPLAYPORT_TX_RCV_DATA_COUNT);
            if (reply_code > 0)
            {
                for (xx = 0; xx < reply_code; xx++)
                {
                    data_buffer[data_index] = dptx_reg_read(XILINX_DISPLAYPORT_TX_RCV_FIFO);
                    data_index++;
                }
                data_count += reply_code;
            }
            else {
                done = 1;
                dbg_printf("Partial read response of %d data received\n\r", status);
            }
        }
        else
        {
            dbg_printf("Error: I2C read failed with code 0x%02x\n\r", status);
            return XILINX_DISPLAYPORT_OPERATION_FAILED;
        }
        //else
        //{
        //    reply_code = dptx_reg_read(XILINX_DISPLAYPORT_TX_RCV_DATA_COUNT);
        //    dbg_printf("Partial read response of %d data received\n\r", status);
        //    for (xx=0; xx < reply_code; xx++)
        //    {
        //        data_buffer[data_index] = dptx_reg_read(XILINX_DISPLAYPORT_TX_RCV_FIFO); // *(volatile UINT32 *)0xb5400134;
        //        data_index++;
        //    }
        //    data_count += reply_code;
        //}
    }
    MB_Sleep(1);
    //xildpWaituS(100);

    return status;
}


/*
 *  Function: xildpSetMSAValues
 *
 *  Update the transmitter MSA values
 *
 *  Parameters:
 *     msa_values - : Pointer to a XILDPMainStreamAttributes struct, must be valid and initialized
 *
 *  Returns:
 *      None
 * 
 *	Example:
 *  (start code)
 *      xildpSetMSAValues(&msa_list);
 *  (end)
 *      
 */
void xildpSetMSAValues(XILDPMainStreamAttributes *msa_values)
{
    XILDPMainLinkConfig *mlconf = dplpmGetMainLinkConfig();

    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_HTOTAL    , msa_values->h_clk_total);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_VTOTAL    , msa_values->v_clk_total);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_POLARITY  , msa_values->v_sync_polarity << 1 | msa_values->h_sync_polarity);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_HSWIDTH   , msa_values->h_sync_width);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_VSWIDTH   , msa_values->v_sync_width);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_HRES      , msa_values->h_resolution);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_VRES      , msa_values->v_resolution);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_HSTART    , msa_values->h_start);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_VSTART    , msa_values->v_start);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_MISC0     , msa_values->misc0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_MISC1     , msa_values->misc1);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_M_VID               , msa_values->m_vid);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_N_VID               , msa_values->n_vid);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_USER_PIXEL_WIDTH    , msa_values->user_pixel_width);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_DATA_PER_LANE       , msa_values->data_per_lane);
    if(mlconf->mst_enable)
    {
    	switch(mlconf->lane_count)
    	{
    		case 1: { dptx_reg_write(XILINX_DISPLAYPORT_TX_TRANSFER_UNIT_SIZE  , 54); break;}
    		case 2: { dptx_reg_write(XILINX_DISPLAYPORT_TX_TRANSFER_UNIT_SIZE  , 28); break;}
    		case 4: { dptx_reg_write(XILINX_DISPLAYPORT_TX_TRANSFER_UNIT_SIZE  , 14); break;}
    	}

    }
    else
    {
        dptx_reg_write(XILINX_DISPLAYPORT_TX_TRANSFER_UNIT_SIZE  , msa_values->transfer_unit_size);
    }
}



/*
 *  Function: xildpDisplayTXRegisters
 *      Print out the contents of all available transmitter core registers
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      None
 * 
 *  Example:
 *  (start code)
 *      xildpDisplayTXRegisters();
 *  (end)
 *      
*/ 
void xildpDisplayTXRegisters(UINT32 show_msa_values)
{
    // TX Regs
    dbg_printf("DisplayPort Transmitter Registers:\n\r");
    dbg_printf("    TX Core ID              : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_CORE_ID));
    dbg_printf("    Link BW                 : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_LINK_BW_SET));
    dbg_printf("    Lane Count              : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_LANE_COUNT_SET));
    dbg_printf("    Enhanced Framing        : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_ENHANCED_FRAME_EN));
    dbg_printf("    Training Pattern        : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_TRAINING_PATTERN_SET));
    dbg_printf("    Link Qual Pattern       : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_LINK_QUAL_PATTERN_SET));
    dbg_printf("    Scrambling Disable      : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_SCRAMBLING_DISABLE));
    dbg_printf("    Downspread Ctrl         : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_DOWNSPREAD_CTRL));
    dbg_printf("    TX Enable               : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_ENABLE));
    dbg_printf("    Main Link Enable        : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_ENABLE_MAIN_STREAM));
    dbg_printf("    Sec Stream Enable       : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_ENABLE_SEC_STREAM));
    dbg_printf("    TX Command Reg          : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_CMD_REG));
    dbg_printf("    TX Address Regs         : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_ADDRESS_REG));
    dbg_printf("    Interrupt Sig           : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_INTERRUPT_SIG_STATE));
    dbg_printf("    Interrupt               : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_INTERRUPT));
    dbg_printf("    Interrupt Mask          : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_INTERRUPT_MASK));
    dbg_printf("    TX Clk Divider          : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_CLK_DIVIDER));
    dbg_printf("    TX Status               : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_STATUS));
    dbg_printf("    TX Reply Reg            : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_RCV_REPLY_REG));
    dbg_printf("    TX Reply Count          : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_RCV_REPLY_COUNT));
    // PHY registers
    dbg_printf("  PHY Registers:\n\r");
    dbg_printf("    Reset                   : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_PHY_RESET));
    dbg_printf("    Preemphasis, Lane 0     : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_LANE_0));
    dbg_printf("    Preemphasis, Lane 1     : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_LANE_1));
    dbg_printf("    Preemphasis, Lane 2     : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_LANE_2));
    dbg_printf("    Preemphasis, Lane 3     : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_PHY_PREEMPHASIS_LANE_3));
    dbg_printf("    Voltage Diff, Lane 0    : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_0));
    dbg_printf("    Voltage Diff, Lane 1    : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_1));
    dbg_printf("    Voltage Diff, Lane 2    : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_2));
    dbg_printf("    Voltage Diff, Lane 3    : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_PHY_VOLTAGE_DIFF_LANE_3));
    dbg_printf("    Transmit PRBS7          : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_PHY_TRANSMIT_PRBS7));
    dbg_printf("    PHY Clock Feedback Set  : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_PHY_CLOCK_FEEDBACK_SETTING));
    dbg_printf("    PHY Status              : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_PHY_STATUS));

    if (show_msa_values)
    {
        // Main Link registers   
        dbg_printf("  Main Stream Attributes\n\r");
        dbg_printf("    Clocks, H Total         : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_HTOTAL));   
        dbg_printf("    Clocks, V Total         : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_VTOTAL));   
        dbg_printf("    Polarity (V / H)        : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_POLARITY)); 
        dbg_printf("    HSync Width             : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_HSWIDTH));  
        dbg_printf("    VSync Width             : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_VSWIDTH));  
        dbg_printf("    Horz Resolution         : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_HRES));     
        dbg_printf("    Vert Resolution         : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_VRES));     
        dbg_printf("    Horz Start              : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_HSTART));   
        dbg_printf("    Vert Start              : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_VSTART));   
        dbg_printf("    Misc0                   : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_MISC0));    
        dbg_printf("    Misc1                   : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_MISC1));    
        dbg_printf("    User Pixel Width        : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_USER_PIXEL_WIDTH));   
        dbg_printf("    M Vid                   : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_M_VID));              
        dbg_printf("    N Vid                   : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_N_VID));              
        dbg_printf("    Transfer Unit Size      : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_TRANSFER_UNIT_SIZE)); 
        dbg_printf("    User Data Count         : 0x%08x\n\r", *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_DATA_PER_LANE));      
    }
}


/*
 *  Function: xildpDisplayMSAValues
 *      Prints out the current MSA values for both the TX and RX cores
 *
 *  Parameters:
 *		None
 *
 *  Returns:
 *      None
 *
 *  Example:
 *  (start code)
 *      xildpDisplayMSAValues();
 *  (end)
 *      
 */
void xildpDisplayMSAValuesTX(void)
{
   dbg_printf(" Main Stream Attributes         TX   \n\r"
          "    Clocks, H Total         : %d     \n\r"
          "    Clocks, V Total         : %d     \n\r"
          "    Polarity (V / H)        : %d     \n\r"
          "    HSync Width             : %d     \n\r"
          "    VSync Width             : %d     \n\r"
          "    Horz Resolution         : %d     \n\r"
          "    Vert Resolution         : %d     \n\r"
          "    Horz Start              : %d     \n\r"
          "    Vert Start              : %d     \n\r"
          "    Misc0                   : 0x%08X     \n\r"
          "    Misc1                   : 0x%08X     \n\r"
          "    User Pixel Width        : %d     \n\r"
          "    M Vid                   : %d     \n\r"
          "    N Vid                   : %d     \n\r"
          "    Transfer Unit Size      : %d\n\r"
          "    User Data Count         : %d\n\r",
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_HTOTAL),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_VTOTAL),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_POLARITY),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_HSWIDTH),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_VSWIDTH),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_HRES),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_VRES),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_HSTART),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_VSTART),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_MISC0),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_MAIN_LINK_MISC1),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_USER_PIXEL_WIDTH),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_M_VID),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_N_VID),
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_TRANSFER_UNIT_SIZE), 
          *(volatile UINT32*)(XILINX_DISPLAYPORT_TX_DATA_PER_LANE)
         );
}

/*void xildpDisplayPatGen (void) {
    dbg_printf(  " Video pattern Generator config \n\r"
                 "    VPOL       : %d \n\r"
                 "    HPOL       : %d \n\r"
                 "    DEPOL      : %d \n\r"
                 "    VSWIDTH    : %d \n\r"
                 "    VB         : %d \n\r"
                 "    VF         : %d \n\r"
                 "    VRES       : %d \n\r"
                 "    HSWIDTH    : %d \n\r"
                 "    HB         : %d \n\r"
                 "    HF         : %d \n\r"
                 "    HRES       : %d \n\r",
    *(volatile UINT32*)(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x04)),
    *(volatile UINT32*)(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x08)),
    *(volatile UINT32*)(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x0C)),
    *(volatile UINT32*)(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x10)),
    *(volatile UINT32*)(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x14)),
    *(volatile UINT32*)(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x18)),
    *(volatile UINT32*)(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x1C)),
    *(volatile UINT32*)(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x20)),
    *(volatile UINT32*)(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x24)),
    *(volatile UINT32*)(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x28)),
    *(volatile UINT32*)(XILINX_DISPLAYPORT_VID_BASE_ADDRESS + (0x2C)));
}*/

/*
 *  Function: xildpResetPHYTX
 *      Resets the transmitter PHY
 *
 *	Parameters:
 *      reset_type - : UINT32 with a value of 1 (master), 2 (functional) or 3 (both)
 * 
 *  Returns:
 *      None
 * 
 *  Example:
 *  (start code)
 *      xildpResetPHYTX(0x01);
 *  (end)
 *      
*/ 
void xildpResetPHYTX(UINT32 reset_type)
{
    //dbg3_printf("Tx PHY reset\n\r");
    dptx_reg_write(XILINX_DISPLAYPORT_TX_ENABLE, 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_RESET, reset_type);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_RESET, 0);
    //dbg3_printf("Waiting for TX PHY...\n\r");
    dptx_wait_phy_ready();
    dptx_reg_write(XILINX_DISPLAYPORT_TX_ENABLE, 1);
}

/*
 *  Function: xildpClearMSAValues
 *      Clears the MSA values from the transmitter registers
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      None
 * 
 *  Example:
 *  (start code)
 *      xildpClearMSAValues();
 *  (end)
 *      
*/ 
void xildpClearMSAValues(void)
{
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_HTOTAL  , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_VTOTAL  , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_POLARITY, 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_HSWIDTH , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_VSWIDTH , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_HRES    , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_VRES    , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_HSTART  , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_VSTART  , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_MISC0   , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_MAIN_LINK_MISC1   , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_TRANSFER_UNIT_SIZE, 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_USER_PIXEL_WIDTH  , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_DATA_PER_LANE     , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_M_VID             , 0);
    dptx_reg_write(XILINX_DISPLAYPORT_TX_N_VID             , 0);
}

