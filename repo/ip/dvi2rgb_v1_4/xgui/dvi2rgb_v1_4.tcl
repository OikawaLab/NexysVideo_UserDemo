
# Loading additional proc with user specified bodies to compute parameter values.
source [file join [file dirname [file dirname [info script]]] gui/dvi2rgb_v1_0.gtcl]

# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  set Component_Name  [  ipgui::add_param $IPINST -name "Component_Name" -display_name {Component Name}]
  set_property tooltip {Component Name} ${Component_Name}
  #Adding Page
  set Page_0  [  ipgui::add_page $IPINST -name "Page 0" -display_name {Page 0}]
  set_property tooltip {Page 0} ${Page_0}
  set kEnableSerialClkOutput  [  ipgui::add_param $IPINST -name "kEnableSerialClkOutput" -parent ${Page_0} -display_name {Enable serial clock output}]
  set_property tooltip {Enable serial clock output} ${kEnableSerialClkOutput}
  set kRstActiveHigh  [  ipgui::add_param $IPINST -name "kRstActiveHigh" -parent ${Page_0} -display_name {Resets active high}]
  set_property tooltip {Resets active high} ${kRstActiveHigh}
  set kEmulateDDC  [  ipgui::add_param $IPINST -name "kEmulateDDC" -parent ${Page_0} -display_name {Enable DDC ROM}]
  set_property tooltip {Enable DDC ROM} ${kEmulateDDC}
  set kClkRange  [  ipgui::add_param $IPINST -name "kClkRange" -parent ${Page_0} -display_name {TMDS clock range} -layout horizontal]
  set_property tooltip {TMDS clock range} ${kClkRange}


}

proc update_PARAM_VALUE.kEnableSerialClkOutput { PARAM_VALUE.kEnableSerialClkOutput } {
	# Procedure called to update kEnableSerialClkOutput when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.kEnableSerialClkOutput { PARAM_VALUE.kEnableSerialClkOutput } {
	# Procedure called to validate kEnableSerialClkOutput
	return true
}

proc update_PARAM_VALUE.kClkRange { PARAM_VALUE.kClkRange } {
	# Procedure called to update kClkRange when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.kClkRange { PARAM_VALUE.kClkRange } {
	# Procedure called to validate kClkRange
	return true
}

proc update_PARAM_VALUE.kRstActiveHigh { PARAM_VALUE.kRstActiveHigh } {
	# Procedure called to update kRstActiveHigh when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.kRstActiveHigh { PARAM_VALUE.kRstActiveHigh } {
	# Procedure called to validate kRstActiveHigh
	return true
}

proc update_PARAM_VALUE.kEmulateDDC { PARAM_VALUE.kEmulateDDC } {
	# Procedure called to update kEmulateDDC when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.kEmulateDDC { PARAM_VALUE.kEmulateDDC } {
	# Procedure called to validate kEmulateDDC
	return true
}


proc update_MODELPARAM_VALUE.kEmulateDDC { MODELPARAM_VALUE.kEmulateDDC PARAM_VALUE.kEmulateDDC } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.kEmulateDDC}] ${MODELPARAM_VALUE.kEmulateDDC}
}

proc update_MODELPARAM_VALUE.kRstActiveHigh { MODELPARAM_VALUE.kRstActiveHigh PARAM_VALUE.kRstActiveHigh } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.kRstActiveHigh}] ${MODELPARAM_VALUE.kRstActiveHigh}
}

proc update_MODELPARAM_VALUE.kClkRange { MODELPARAM_VALUE.kClkRange PARAM_VALUE.kClkRange } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.kClkRange}] ${MODELPARAM_VALUE.kClkRange}
}

proc update_MODELPARAM_VALUE.kIDLY_TapValuePs { MODELPARAM_VALUE.kIDLY_TapValuePs } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "kIDLY_TapValuePs". Setting updated value from the model parameter.
set_property value 78 ${MODELPARAM_VALUE.kIDLY_TapValuePs}
}

proc update_MODELPARAM_VALUE.kIDLY_TapWidth { MODELPARAM_VALUE.kIDLY_TapWidth } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	# WARNING: There is no corresponding user parameter named "kIDLY_TapWidth". Setting updated value from the model parameter.
set_property value 5 ${MODELPARAM_VALUE.kIDLY_TapWidth}
}

