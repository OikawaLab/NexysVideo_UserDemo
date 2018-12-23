# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
	set Component_Name [ipgui::add_param $IPINST -name Component_Name]
	set Page0 [ipgui::add_page $IPINST -name "Page 0" -layout vertical]
	set kDataWidth [ipgui::add_param $IPINST -parent $Page0 -name kDataWidth]
}

proc update_PARAM_VALUE.kDataWidth { PARAM_VALUE.kDataWidth } {
	# Procedure called to update kDataWidth when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.kDataWidth { PARAM_VALUE.kDataWidth } {
	# Procedure called to validate kDataWidth
	return true
}


proc update_MODELPARAM_VALUE.kDataWidth { MODELPARAM_VALUE.kDataWidth PARAM_VALUE.kDataWidth } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.kDataWidth}] ${MODELPARAM_VALUE.kDataWidth}
}

