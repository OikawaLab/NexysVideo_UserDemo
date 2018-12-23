### This constraint file's processing order must be set to LATE, to make sure clocks are defined first

# Use fastest clock of the BUFGMUX as the path that timing analysis should be done on
# In this case, both clocks have the same period constraint, so either input 0 or input 1 can be selected
#set_case_analysis 0 [get_pins design_3_i/vid_io_mux_0/aSel]

# Ignore paths between invalid master-generated clock combinations
# The PixelClk input to rgb2dvi has two clocks defined on it, because of the BUFGMUX: HDMIOutClk0 and HDMIOutClk1
# These go through the PLL in rgb2dvi, generating PixelClkIO_2 (HDMIOutClk0 as master) and PixelClkIO_3 (HDMIOutClk1 as master)
# PixeClkIO_2 - HDMIOutClk1 and PixeClkIO_3 - HDMIOutClk0 pairs are invalid
set_clock_groups -name gen_from_hdmioutmux -asynchronous -group [get_clocks -include_generated_clocks HDMIOutClk0] -group [get_clocks -include_generated_clocks HDMIOutClk1]

set_clock_groups -name gen_from_dpoutmux -asynchronous -group [get_clocks -include_generated_clocks DPOutClk0] -group [get_clocks -include_generated_clocks DPOutClk1]

## Vivado bug workaround. IP XDCs don't get applied for both vid_io_mux instances
### The aSel signal is allowed to violate setup/hold times. ###
### The worst that can happen is an extra clock pulse from the old clock ###
set_false_path -through [get_pins {system_i/video/vid_io_mux_dp/U0/PixelClkMux/S0 system_i/video/vid_io_mux_dp/U0/PixelClkMux/S1}]
set_false_path -through [get_pins {system_i/video/vid_io_mux_dvi/U0/PixelClkMux/S0 system_i/video/vid_io_mux_dvi/U0/PixelClkMux/S1}]

### Asynchronous clock domain crossings ###
set_false_path -through [get_pins -filter {NAME =~ system_i/video/vid_io_mux_dp/*SyncAsync*/oSyncStages_reg[0]/D} -hier]
set_false_path -through [get_pins -filter {NAME =~ system_i/video/vid_io_mux_dvi/*SyncAsync*/oSyncStages_reg[0]/D} -hier]
