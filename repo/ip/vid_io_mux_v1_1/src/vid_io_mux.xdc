### Clock definitions ###
# Define both clocks on mux output, so they are both analyzed.
# Make sure however, that they are marked physically_exclusive, since they don't exist AT THE SAME TIME

# BEGIN COPY TO NORMAL PROJECT XDC
# create_generated_clock -name OutClk0 -divide_by 1 -source [get_pins PixelClkMux/I0] [get_pins PixelClkMux/O]
# create_generated_clock -name OutClk1 -divide_by 1 -source [get_pins PixelClkMux/I1] [get_pins PixelClkMux/O] -add -master_clock [get_clocks -of_objects [get_pins PixelClkMux/I1]]
# set_clock_groups -physically_exclusive -group OutClk0 -group OutClk1
# END COPY TO NORMAL PROJECT XDC

# BEGIN COPY TO LATE PROJECT XDC
# set_clock_groups -name gen_from_hdmioutmux -asynchronous -group [get_clocks -include_generated_clocks OutClk0] -group [get_clocks -include_generated_clocks OutClk1]
# END COPY TO LATE PROJECT XDC

### The aSel signal is allowed to violate setup/hold times. ###
### The worst that can happen is an extra clock pulse from the old clock ###
set_false_path -through [get_pins {PixelClkMux/S0 PixelClkMux/S1}]

### Asynchronous clock domain crossings ###
set_false_path -through [get_pins -filter {NAME =~ *SyncAsync*/oSyncStages_reg[0]/D} -hier]