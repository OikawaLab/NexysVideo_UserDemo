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
#include "displayport_tx_drv.h"
#include "sys_defs.h"
#include "displayport_defs.h"
#include "xlib_string.h"
#include "xil_displayport.h"
#include "microblaze_sleep.h"

/*
 *  Function: dptx_init
 *      Initialize the Displayport transmitter
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      UINT32, zero for success, non-zero for failure
 * 
 *	Example:
 *      dptx_init();
 *      
*/ 
UINT32 dptx_init(void)
{
    UINT32 status = XILINX_DISPLAYPORT_OPERATION_SUCCESS;

    // Disable the transmitter
    dptx_reg_write(XILINX_DISPLAYPORT_TX_ENABLE, 0x00);
    // Put the PHY into reset
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_RESET, XILINX_DISPLAYPORT_PHY_RESET_ALL);
    // Set the clock divider
    dptx_reg_write(XILINX_DISPLAYPORT_TX_CLK_DIVIDER, SYSDEF_AUX_CLOCK_SPEED);
    // Set Displayport clock speed
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_CLOCK_FEEDBACK_SETTING, XILINX_DISPLAYPORT_TX_PHY_CLK_FB_270);
    // Bring the PHY out of reset
    dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_RESET, 0x00);
    // Wait for the PHY to be ready
    dptx_wait_phy_ready();
    // Enable the transmitter
    dptx_reg_write(XILINX_DISPLAYPORT_TX_ENABLE, 0x01);
    // Unmask all interrupts
    dptx_reg_write(XILINX_DISPLAYPORT_TX_INTERRUPT_MASK, 0x00);

    return status;
}

/*
 *  Function: dptx_set_clkspeed
 *      Sets the clock frequency for Displayport PHY
 *
 *	Parameters:
 *      speed - UINT32, must be one of the following constants:
 *                 XILINX_DISPLAYPORT_TX_PHY_CLK_FB_270
 *                 XILINX_DISPLAYPORT_TX_PHY_CLK_FB_162\
 *              Other values will be ignored
 *
 *  Returns:
 *      UINT32, zero on success, non-zero on failure
 * 
 *	Example:
 *      status = dptx_set_clkspeed(XILINX_DISPLAYPORT_TX_PHY_CLK_FB_270);
 *      
*/ 
UINT32 dptx_set_clkspeed(UINT32 speed)
{
    UINT32 status = 0;

    // Disable TX core
    dptx_reg_write(XILINX_DISPLAYPORT_TX_ENABLE, 0x00);


    // Change speed only if a valid value is specified
    if (speed == XILINX_DISPLAYPORT_TX_PHY_CLK_FB_270 || 
        speed == XILINX_DISPLAYPORT_TX_PHY_CLK_FB_162 ||
        speed == XILINX_DISPLAYPORT_TX_PHY_CLK_FB_540)
    {
        dptx_reg_write(XILINX_DISPLAYPORT_TX_PHY_CLOCK_FEEDBACK_SETTING, speed);
    }
    else
        status = XILINX_DISPLAYPORT_OPERATION_FAILED;


    // Enale TX core
    dptx_reg_write(XILINX_DISPLAYPORT_TX_ENABLE, 0x01);
    // Wait for the PHY to be ready
    status = dptx_wait_phy_ready();
    return status;
}

/*
 *  Function: dptx_tx_ready
 *      Gets the current transmitter status and checks to see if a request is
 *      currently in progress
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      UINT32, zero indicates ready (no request in progress)
 *              non-zero indicates not ready (request in progress)
 * 
 *	Example:
 *      if (dptx_tx_ready() == 0)
 *          send_new_request();
 *      else
 *          printf("Request in progress\n\r");
 *      
*/ 
UINT32 dptx_tx_ready(void)
{
    return !(dptx_reg_read(XILINX_DISPLAYPORT_TX_STATUS) & XILINX_DISPLAYPORT_TX_AUX_REQUEST_IN_PROGRESS);
}

/*
 *  Function: dptx_send_request
 *      Send a request over the Displayport AUX channel
 *
 *	Parameters:
 *      req - XILDPAUXTransaction pointer, the request to send to the sink device
 *            Note that for write requests, the wr_data pointer in the transaction
 *            structure must be assigned to a valid memory region before being
 *            passed to this function
 * 
 *  Returns:
 *      UINT32, zero on success, non-zero on failure
 * 
 *  Example:
 *      XILDPAUXTransaction the_transaction;
 *      UINT8 write_data[16];
 *      the_transaction.wr_data = &write_data[0];
 *      dptx_send_request(&the_transaction);
 *      
*/ 
UINT32 dptx_send_request( XILDPAUXTransaction* req )
{
    UINT32 status = XILINX_DISPLAYPORT_OPERATION_SUCCESS;
    UINT32 xx = 0;
    while ( !dptx_tx_ready() )
    {
//        xil_printf("[XILINX_DISPLAYPORT_TX_AUX_REQUEST_IN_PROGRESS] 0x%x\r\n", dptx_reg_read(XILINX_DISPLAYPORT_TX_STATUS) & XILINX_DISPLAYPORT_TX_AUX_REQUEST_IN_PROGRESS);
    }
    // Set address
    dptx_reg_write(XILINX_DISPLAYPORT_TX_ADDRESS_REG, req->address);
    if (req->cmd_code == XILINX_DISPLAYPORT_CMD_REQUEST_WRITE        || 
        req->cmd_code == XILINX_DISPLAYPORT_CMD_REQUEST_I2C_WRITE    ||      
        req->cmd_code == XILINX_DISPLAYPORT_CMD_REQUEST_I2C_WRITE_MOT )
    {

        for (xx = 0; xx < req->num_bytes; xx++){
//            dbg_printf(" 0x%02x ",req->wr_data[xx]);
            dptx_reg_write(XILINX_DISPLAYPORT_TX_FIFO, req->wr_data[xx]);
        }
    } else {
//        dbg_printf("rd addr [0x%04x]     ",req->address);
    }
    // Submit the command and the data size
    dptx_reg_write(XILINX_DISPLAYPORT_TX_CMD_REG, (req->cmd_code | ((req->num_bytes - 1) & 0x0F)));
    
    return status;
}

/*
 *  Function: dptx_read_reply
 *      Reads the reply code and data, if necessary, in response to a request
 *
 *	Parameters:
 *      is_read    - boolean, specifies a read or write transaction
 *      byte_count - UINT32, number of bytes expected to be received
 *      reply      - XILDPAUXTransaction pointer, the transaction structure to fill out
 *                   with the data from the reply. Note that the rd_data field is a
 *                   pointer and must have memory assigned to it before being passed
 *                   into this function.
 * 
 *  Returns:
 *      UINT32, the reply code received
 * 
 *  Example:
 *      XILDPAUXTransaction the_transaction;
 *      UINT8 read_data[16];
 *      the_transaction.rd_data = &read_data[0];
 *      dptx_read_reply(true, 8, &the_transaction);
 *      
*/ 
UINT32 dptx_read_reply(Xboolean is_read, UINT32 byte_count, XILDPAUXTransaction *reply)
{
    UINT32 xx = 0;

    // Get the reply code - only the low 2 bits matter
    reply->cmd_code = dptx_reg_read(XILINX_DISPLAYPORT_TX_RCV_REPLY_REG) & 0x03;

    // Check for a read command
    if (is_read == TRUE)
    {
        // If ACK'd, read the data out
        if (reply->cmd_code == XILINX_DISPLAYPORT_CMD_REPLY_ACK)
        {
            // Get the number of bytes the transmitter received
            reply->num_bytes = dptx_reg_read(XILINX_DISPLAYPORT_TX_RCV_DATA_COUNT);
            // Verify byte count
            if (reply->num_bytes != byte_count)
            {    
                // Overwrite the command code here
                reply->cmd_code = XILINX_DISPLAYPORT_CMD_REPLY_INVALID;
            }
            else
            {
                // Read data out of the FIFO
                for (xx = 0; xx < byte_count; xx++)
                    reply->rd_data[xx] = dptx_reg_read(XILINX_DISPLAYPORT_TX_RCV_FIFO);
            }
        }
    }
    else
    {
        // If the write gets a nack, need to report the number of bytes actually written
        if (reply->cmd_code == XILINX_DISPLAYPORT_CMD_REPLY_NACK)
        {
            if ( dptx_reg_read(XILINX_DISPLAYPORT_TX_RCV_DATA_COUNT) > 0)
                reply->num_bytes = dptx_reg_read(XILINX_DISPLAYPORT_TX_RCV_FIFO);
        }
    }
    // Return status
    return reply->cmd_code;
}

/*
 *  Function: dptx_enable
 *      Enable or disable the transmitter core
 *
 *	Parameters:
 *      enable - boolean, zero to disable, non-zero to enable
 * 
 *  Returns:
 *      None
 * 
 *	Example:
 *      dptx_enable(0x01);
 *      
*/ 
void dptx_enable(Xboolean enable)
{
    dptx_reg_write(XILINX_DISPLAYPORT_TX_ENABLE, enable);
}

/*
 *  Function: dptx_wait_phy_ready
 *      Waits for the Displayport PHY to come out of reset. This is a blocking function
 *      call.
 *
 *	Parameters:
 *      None     
 * 
 *  Returns:
 *      UINT32, zero on success, non-zero on failure
 * 
 *	Example:
 *      dptx_wait_phy_ready();
 *      
*/ 
UINT32 dptx_wait_phy_ready(void)
{
    UINT32 status = 0;
    UINT32 done = 100 * SYSDEF_ONE_MICROSECOND;

    // Read the status register
    status = dptx_reg_read(XILINX_DISPLAYPORT_TX_PHY_STATUS);
    // Wait for PHY to be ready
    while ((status & 0x3F) != 0x3F)// && done > 0)
    {
    	MB_Sleep(1);
    	//xildpWaituS(1);
        status = dptx_reg_read(XILINX_DISPLAYPORT_TX_PHY_STATUS);
        done--;
    }
    return status;
}

/*
 *  Function: 
 *      Wait for a reply to be received
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      XILINX_DISPLAYPORT_TX_AUX_REPLY_RECEIVED if a valid reply is received
 *      XILINX_DISPLAYPORT_OPERATION_TIMEOUT if the response time period is exceeded
 * 
 *	Example:
 *      dptx_wait_reply();
 *      
*/ 
UINT32 dptx_wait_reply(void)
{
    UINT32 status = 99;
    UINT32 state_value = 0;
    UINT32 timeout_value = 0;

    //dbg4_printf("dptx_wait_reply\n\r");

    while(status == 99 && timeout_value < (2 * SYSDEF_ONE_MILLISECOND))
    {
        state_value = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT_SIG_STATE);
        // check for reply received
        if (state_value & XILINX_DISPLAYPORT_TX_AUX_REPLY_RECEIVED)
            status = XILINX_DISPLAYPORT_OPERATION_SUCCESS; // reply received
        // Check for transaction timeout condition
        if (state_value & XILINX_DISPLAYPORT_TX_AUX_REPLY_TIMEOUT)
            status = XILINX_DISPLAYPORT_OPERATION_TIMEOUT; // timeout
        timeout_value++;
        //dbg_printf(".");
    }
    // Return final transaction status
    return status;
}

/*
 *  Function: dptx_reg_read
 *      Read the contents of a register
 *
 *	Parameters:
 *      reg_address - UINT32, the address of the register to read
 * 
 *  Returns:
 *      UINT32, value of the register
 * 
 *	Example:
 *      UINT32 num_lanes = dptx_reg_read(XILINX_DISPLAYPORT_TX_LANE_COUNT_SET);
 *      
*/ 
UINT32 dptx_reg_read(UINT32 reg_address)
{
	//dbg_printf("dptx_reg_read (0x%08x) ", reg_address);
	UINT32 data = *(volatile UINT32*)reg_address;
	//dbg_printf(" 0x%08x\n\r", data);
    return data;
}

/*
 *  Function: dptx_reg_write
 *      Assign a new value to a register 
 *
 *	Parameters:
 *      reg_address - UINT32, register address to write
 *      data        - UINT32, value to assign to the register
 * 
 *  Returns:
 *      None
 * 
 *	Example:
 *      dptx_reg_read(XILINX_DISPLAYPORT_TX_LANE_COUNT_SET, 4);
 *      
*/ 
void dptx_reg_write(UINT32 reg_address, UINT32 data)
{
	//dbg_printf("dptx_reg_write (0x%08x) 0x%08x\n\r", reg_address, data);
    *(volatile UINT32*)reg_address = data;
}

/*
 *  Function: dptx_isr
 *      Transmitter core interrupt service routine
 *
 *	Parameters:
 *      arg0     - UINT32, number of data elements
 *      isr_data - void pointer, list of data elements 
 * 
 *  Returns:
 *      UINT32, zero on success, non-zero on failure
 * 
 *	Example:
 *      if (interrupt)
 *          dptx_isr(isr_arg_count, user_data);
 *      
*/ 
UINT32 dptx_isr(UINT32 arg0, void *isr_data)
{
    // Read the interrupt register
    UINT32 status = dptx_reg_read(XILINX_DISPLAYPORT_TX_INTERRUPT);
    // Return status
    return status;
}

/*
 *  Function: dptx_get_reply_code
 *      Convenience function for getting the last reply code
 *
 *	Parameters:
 *      None
 * 
 *  Returns:
 *      None
 * 
 *	Example:
 *      UINT32 last_code = dptx_get_reply_code();
 *      
*/ 
UINT32 dptx_get_reply_code(void)
{
    return dptx_reg_read(XILINX_DISPLAYPORT_TX_RCV_REPLY_REG);
}

/*
 *  Function: dptx_set_user_data_count
 *      Program the User Data Count register for the configured resolution
 *
 *	Parameters:
 *      hres - : Horizontal resolution
 *      bytes_per_pixel - : Number of bytes per pixel
 *      lane_count - : Configured number of lanes
 * 
 *  Returns:
 *      UINT32 - the user data count that was set
 * 
 *	Example:
 *      
 *      
*/ 
UINT32 dptx_set_user_data_count(UINT32 hres, UINT32 bytes_per_pixel, UINT32 lane_count)
{
    UINT32 udc = 0;
    
    // Compute user data count (data per lane)    
    udc = ((hres) * bytes_per_pixel) >> 1;
    // Actual value of HW register is 1 less than computed value
    udc--;
    // Program the value into the hardware
    dptx_reg_write(XILINX_DISPLAYPORT_TX_DATA_PER_LANE, udc);
    // Return the User data count
    return udc;
}

