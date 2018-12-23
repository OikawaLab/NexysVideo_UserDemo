#=============================================================================
# This script programs an MCS or BIN file to the Flash on the Atlys 2 and
# enables quad mode
#=============================================================================
set flash_img "./nexys_video_demo.mcs"
set script_path [file dirname [info script]]

# Do not modify anything below this line
connect_hw_server -quiet

current_hw_target [get_hw_targets */Digilent/*] -quiet
open_hw_target -quiet
get_hw_devices -quiet
refresh_hw_device [lindex [get_hw_devices] 0] -quiet

# Set memory part and parameters
create_hw_cfgmem -hw_device [lindex [get_hw_devices] 0] -mem_dev  [lindex [get_cfgmem_parts {s25fl256sxxxxxx0-spi-x1_x2_x4}] 0]
refresh_hw_device [lindex [get_hw_devices] 0]
set_property PROGRAM.ADDRESS_RANGE  {use_file} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
set_property PROGRAM.FILES [list $flash_img ] [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0]]
set_property PROGRAM.UNUSED_PIN_TERMINATION {pull-none} [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
set_property PROGRAM.BLANK_CHECK  0 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
set_property PROGRAM.ERASE  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
set_property PROGRAM.CFG_PROGRAM  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
set_property PROGRAM.VERIFY  1 [ get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]
if {![string equal [get_property PROGRAM.HW_CFGMEM_TYPE  [lindex [get_hw_devices] 0]] [get_property MEM_TYPE [get_property CFGMEM_PART [get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]]]] }  { create_hw_bitstream -hw_device [lindex [get_hw_devices] 0] [get_property PROGRAM.HW_CFGMEM_BITFILE [ lindex [get_hw_devices] 0]]; program_hw_devices [lindex [get_hw_devices] 0]; };

# Program Flash
program_hw_cfgmem -hw_cfgmem [get_property PROGRAM.HW_CFGMEM [lindex [get_hw_devices] 0 ]]

# Set Quad Enable bit
refresh_hw_device [lindex [get_hw_devices] 0]
current_hw_core [lindex [get_hw_core] 0]
source $script_path/prog_spi_quad_enable.tcl -quiet
xspi_set_spi_clk
xspi_read_id
xspi_prog_quad_enable 1
xspi_read_statregs