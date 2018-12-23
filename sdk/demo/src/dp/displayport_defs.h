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


#ifndef __XILINX_DISPLAYPORT_DEFS__
#define __XILINX_DISPLAYPORT_DEFS__

typedef unsigned char   UINT8;  /**< unsigned 8-bit */
typedef unsigned short  UINT16; /**< unsigned 16-bit */
typedef unsigned int    UINT32; /**< unsigned 32-bit */

typedef signed char     INT8;  /**< unsigned 8-bit */
typedef signed short    INT16; /**< unsigned 16-bit */
typedef signed int      INT32; /**< unsigned 32-bit */

#define VIRTEX6 0
#define KINTEX7 1

// AUX CH Requests
#define XILINX_DISPLAYPORT_CMD_REQUEST_READ                      0x0900
#define XILINX_DISPLAYPORT_CMD_REQUEST_WRITE                     0x0800
#define XILINX_DISPLAYPORT_CMD_REQUEST_I2C_READ                  0x0100
#define XILINX_DISPLAYPORT_CMD_REQUEST_I2C_WRITE                 0x0000
#define XILINX_DISPLAYPORT_CMD_REQUEST_I2C_WRITE_STATUS          0x0200
#define XILINX_DISPLAYPORT_CMD_REQUEST_I2C_READ_MOT              0x0500
#define XILINX_DISPLAYPORT_CMD_REQUEST_I2C_WRITE_MOT             0x0400
#define XILINX_DISPLAYPORT_CMD_REQUEST_I2C_WRITE_STATUS_MOT      0x0600

// AUX Channel Replies
#define XILINX_DISPLAYPORT_CMD_REPLY_ACK                         0x00
#define XILINX_DISPLAYPORT_CMD_REPLY_NACK                        0x01
#define XILINX_DISPLAYPORT_CMD_REPLY_DEFER                       0x02
#define XILINX_DISPLAYPORT_CMD_REPLY_UNKNOWN                     0xAA
#define XILINX_DISPLAYPORT_CMD_REPLY_INVALID                     0x55
#define XILINX_DISPLAYPORT_CMD_REPLY_I2C_ACK                     0x00
#define XILINX_DISPLAYPORT_CMD_REPLY_I2C_NACK                    0x04
#define XILINX_DISPLAYPORT_CMD_REPLY_I2C_DEFER                   0x08
#define XILINX_DISPLAYPORT_CMD_ADDRESS_ONLY                      0x1000

// Test Response Values
#define XILINX_DISPLAYPORT_TEST_RESPONSE_ACK                     0x01
#define XILINX_DISPLAYPORT_TEST_RESPONSE_NACK                    0x02
#define XILINX_DISPLAYPORT_TEST_RESPONSE_WR_CHECKSUM             0x04

// Displayport Core Device Type
#define XILINX_DISPLAYPORT_DEVICE_TYPE_TRANSMITTER               0x01
#define XILINX_DISPLAYPORT_DEVICE_TYPE_RECEIVER                  0x02
#define XILINX_DISPLAYPORT_DEVICE_TYPE_TRANSCEIVER               0x03

// Operational Return / Status Values
#define XILINX_DISPLAYPORT_OPERATION_SUCCESS                     0x00000000
#define XILINX_DISPLAYPORT_OPERATION_FAILED                      0xFFFFFFFF
#define XILINX_DISPLAYPORT_OPERATION_TIMEOUT                     0x0000AABB
#define XILINX_DISPLAYPORT_OPERATION_REQUEST_IN_PROGRESS         0x0000AABC
#define XILINX_DISPLAYPORT_OPERATION_DEFER                       0x0000AABD
#define XILINX_DISPLAYPORT_INVALID_REPLY_CODE                    0x0000BB33
#define XILINX_DISPLAYPORT_INVALID_PARAMETER                     0x0000BB22
#define XILINX_DISPLAYPORT_NOT_CONNECTED                         0x0000BB11

// Miscellaneous Constants
#define XILINX_DISPLAYPORT_MAX_RETRY_COUNT                       1
#define XILINX_DISPLAYPORT_DEFAULT_TIMEOUT_VALUE                 (500 * SYSDEF_ONE_MICROSECOND)
#define XILINX_DISPLAYPORT_MAX_FIFO_DEPTH                        16

#define XILINX_DISPLAYPORT_LINK_RATE_1_62GBPS                    0x06
#define XILINX_DISPLAYPORT_LINK_RATE_2_7GBPS                     0x0A
#define XILINX_DISPLAYPORT_LINK_RATE_5_4GBPS                     0x14

// Enable Bits / Flags
#define XILINX_DISPLAYPORT_ENABLE_HDCP                           0x01
#define XILINX_DISPLAYPORT_ENABLE_TX                             0x02
#define XILINX_DISPLAYPORT_ENABLE_RX                             0x03
#define XILINX_DISPLAYPORT_ENABLE_MAIN_LINK                      0x04 
#define XILINX_DISPLAYPORT_ENABLE_SEC_LINK                       0x05

#define XILINX_DISPLAYPORT_TRAINING_OFF                          0x00
#define XILINX_DISPLAYPORT_TRAINING_PATTERN_1                    0x21
#define XILINX_DISPLAYPORT_TRAINING_PATTERN_2                    0x22
#define XILINX_DISPLAYPORT_TRAINING_PATTERN_3                    0x23
#define XILINX_DISPLAYPORT_LINK_QUAL_PATTERN_OFF                 0x00
#define XILINX_DISPLAYPORT_LINK_QUAL_PATTERN_D10_2               0x24
#define XILINX_DISPLAYPORT_LINK_QUAL_PATTERN_SYM_ERR             0x08
#define XILINX_DISPLAYPORT_LINK_QUAL_PATTERN_PRBS7               0x2C

#define XILINX_DISPLAYPORT_LINK_TRAINING_TIMER                   5
#define XILINX_DISPLAYPORT_MAX_TRAINING_TIME                     (100 * SYSDEF_ONE_MILLISECOND)
#define XILINX_DISPLAYPORT_TRAINING_SETTINGS_ALL                 0xFF

#define XILINX_DISPLAYPORT_POWER_STATE_ON                        0x01
#define XILINX_DISPLAYPORT_POWER_STATE_PWRSAVE                   0x02

#define XILINX_DISPLAYPORT_ENHANCED_FRAMING_MODE_BIT             0x80
#define XILINX_DISPLAYPORT_DOWNSPREAD_ENABLE_BIT                 0x10

#define XILINX_DISPLAYPORT_PHY_RESET_MASTER                      0x01
#define XILINX_DISPLAYPORT_PHY_RESET_FUNCTIONAL                  0x02
#define XILINX_DISPLAYPORT_PHY_RESET_ALL                         0x03

#define XILINX_DISPLAYPORT_DEVICE_SERVICE_IRQ_REMOTE_CMD_PEND    0x01
#define XILINX_DISPLAYPORT_DEVICE_SERVICE_IRQ_TEST               0x02
#define XILINX_DISPLAYPORT_DEVICE_SERVICE_IRQ_CP                 0x04
#define XILINX_DISPLAYPORT_DEVICE_SERVICE_IRQ_VENDOR             0x40


// Source request transaction
// Max data size = 16 bytes
typedef struct 
{
    UINT16 cmd_code;                                        // Command for request, reply code for reply
    UINT8  num_bytes;                                       // Number of bytes of read/write data
    UINT32 address;                                         // 20-bit AUX address
    UINT8  *rd_data;                                        // Recevied data for read command
    UINT8  *wr_data;                                        // Pointer to write data (max of 16 bytes)
    UINT8  exit_when_defer;                                 // Used in Test Mode
}XILDPAUXTransaction;

#endif /* __XILINX_DISPLAYPORT_DEFS__ */
