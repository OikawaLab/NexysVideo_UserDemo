# Dummy clock definitions when synthesizing out-of-context
create_clock -period 10.000 -name PixelClk_0 -waveform {0.000 5.000} [get_ports PixelClk_0]
create_clock -period 10.000 -name PixelClk_1 -waveform {0.000 5.000} [get_ports PixelClk_1]
