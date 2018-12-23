# Rename auto-generated clocks from clk_wiz_0
# Reference clock 200 MHz
create_generated_clock -name clk200_ref [get_pins system_i/mig_7series_0/u_system_mig_7series_0_0_mig/u_ddr3_infrastructure/gen_ui_extra_clocks.mmcm_i/CLKOUT1]
# Axi-lite & Microblaze clock 100 MHz
create_generated_clock -name clk100_axi [get_pins system_i/mig_7series_0/u_system_mig_7series_0_0_mig/u_ddr3_infrastructure/gen_ui_extra_clocks.mmcm_i/CLKFBOUT]
# Internal fixed pixel clock 108 MHz
create_generated_clock -name clk108_pix [get_pins system_i/clk_wiz_0/U0/mmcm_adv_inst/CLKOUT1]

# Audio forwarded clock
create_generated_clock -name AC_MCLK -source [get_pins system_i/d_axi_i2s_audio_0/U0/d_axi_i2s_audio_v2_0_AXI_L_inst/Inst_I2sCtl/ODDR_inst/C] -divide_by 1 [get_ports AC_MCLK]

# Make sure clk108_pix BUFG is placed between the two BUFGMUXs of the vid_io_mux cores
# Placement cannot seem to figure this out by itself
set_property LOC MMCME2_ADV_X1Y2 [get_cells system_i/video/dvi2rgb_0/U0/TMDS_ClockingX/DVI_ClkGenerator]
set_property LOC MMCME2_ADV_X0Y2 [get_cells system_i/clk_wiz_0/U0/mmcm_adv_inst]
set_property LOC BUFGCTRL_X0Y3 [get_cells system_i/clk_wiz_0/U0/clkout2_buf]
set_property LOC BUFGCTRL_X0Y4 [get_cells system_i/video/vid_io_mux_dp/U0/PixelClkMux]
set_property LOC BUFGCTRL_X0Y2 [get_cells system_i/video/vid_io_mux_dvi/U0/PixelClkMux]

# Propagate clocks through BUFGMUX
create_generated_clock -name HDMIOutClk0 -source [get_pins system_i/video/vid_io_mux_dvi/U0/PixelClkMux/I0] -divide_by 1 [get_pins system_i/video/vid_io_mux_dvi/U0/PixelClkMux/O]
create_generated_clock -name HDMIOutClk1 -source [get_pins system_i/video/vid_io_mux_dvi/U0/PixelClkMux/I1] -divide_by 1 -add -master_clock PixelClk_1 [get_pins system_i/video/vid_io_mux_dvi/U0/PixelClkMux/O]

create_generated_clock -name DPOutClk0 -source [get_pins system_i/video/vid_io_mux_dp/U0/PixelClkMux/I0] -divide_by 1 [get_pins system_i/video/vid_io_mux_dp/U0/PixelClkMux/O]
create_generated_clock -name DPOutClk1 -source [get_pins system_i/video/vid_io_mux_dp/U0/PixelClkMux/I1] -divide_by 1 -add -master_clock PixelClk_1 [get_pins system_i/video/vid_io_mux_dp/U0/PixelClkMux/O]

# Clock exclusions will be set by vid_io_mux_clocks constraints
set_clock_groups -name hdmi_async_clk01 -asynchronous -group [get_clocks -of_objects [get_pins system_i/video/vid_io_mux_dvi/U0/PixelClkMux/I0]] -group HDMIOutClk1
set_clock_groups -name hdmi_async_clk10 -asynchronous -group [get_clocks -of_objects [get_pins system_i/video/vid_io_mux_dvi/U0/PixelClkMux/I1]] -group HDMIOutClk0

set_clock_groups -name dp_async_clk01 -asynchronous -group [get_clocks -of_objects [get_pins system_i/video/vid_io_mux_dp/U0/PixelClkMux/I0]] -group DPOutClk1
set_clock_groups -name dp_async_clk10 -asynchronous -group [get_clocks -of_objects [get_pins system_i/video/vid_io_mux_dp/U0/PixelClkMux/I1]] -group DPOutClk0

