# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  set Component_Name  [  ipgui::add_param $IPINST -name "Component_Name" -display_name {Component Name}]
  set_property tooltip {Component Name} ${Component_Name}
  #Adding Page
  set Page_0  [  ipgui::add_page $IPINST -name "Page 0" -display_name {Page 0}]
  set_property tooltip {Page 0} ${Page_0}
  set kPolarity  [  ipgui::add_param $IPINST -name "kPolarity" -parent ${Page_0} -display_name {Reset Polarity}]
  set_property tooltip {Reset Polarity} ${kPolarity}


}

proc update_PARAM_VALUE.kPolarity { PARAM_VALUE.kPolarity } {
	# Procedure called to update kPolarity when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.kPolarity { PARAM_VALUE.kPolarity } {
	# Procedure called to validate kPolarity
	return true
}


proc update_MODELPARAM_VALUE.kPolarity { MODELPARAM_VALUE.kPolarity PARAM_VALUE.kPolarity } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.kPolarity}] ${MODELPARAM_VALUE.kPolarity}
}

