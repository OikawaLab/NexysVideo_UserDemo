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


#ifndef __XILINX_SYS_DEFS__
#define __XILINX_SYS_DEFS__

#include "xparameters.h"

// -------------------------------------------------------------------------------------------------
// Register Base Addresses
// -------------------------------------------------------------------------------------------------
#define XILINX_DISPLAYPORT_TX_BASE_ADDRESS   XPAR_VIDEO_DISPLAYPORT_0_BASEADDR


#define XILINX_CORE_ID_DISPLAYPORT_TX        0x00
#define XILINX_CORE_ID_DISPLAYPORT_RX        0x01

// -------------------------------------------------------------------------------------------------
// Timer reference values
// -------------------------------------------------------------------------------------------------
#define SYSDEF_AUX_CLOCK_SPEED                  (XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ / 1000000)
#define SYSDEF_ONE_MICROSECOND                  (SYSDEF_AUX_CLOCK_SPEED*100)
#define SYSDEF_ONE_MILLISECOND                  (SYSDEF_AUX_CLOCK_SPEED*1000)
#define SYSDEF_ONE_SECOND                       (SYSDEF_AUX_CLOCK_SPEED*1000000)


#endif /* __XILINX_SYS_DEFS__ */

