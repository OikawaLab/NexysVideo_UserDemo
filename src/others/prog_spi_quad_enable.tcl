#=============================================================================
# 2014/05/31: Program Spansion SPI quad enable bit
# - to test:
#   1. Program 7-series FPGA with SPI core
#   2. In tcl console, type "current_hw_core [lindex [get_hw_core] 0]"
#   3. Source this script
#   4. Run "xspi_set_spi_clk"
#   5. Run "xspi_read_id" to read device ID
#   6. Run "xspi_prog_quad_enable 1" to program config register
#      quad enable bit, "xspi_prog_quad_enable 0" to clear quad enable bit
#   7. Run "xspi_read_statregs" to read status and configuration status
#      registers
#-----------------------------------------------------------------------------
# Revision history:
# 2014/05/31: Script derived from UltraScale Quad SPI tcl file
#-----------------------------------------------------------------------------
#=============================================
# Notes:
#=============================================
# XSDB register read order:
# After an XSDB red operation, the data in the register is stored as
# follows:
#   Word 0 = Byte 1 + Byte 0
#   Word 1 = Byte 3 + Byte 2
#   Word 2 = Byte 5 + Byte 4
#   Word 3 = Byte 7 + Byte 6
#
# Ex: When reading the N25Q128 SPI device IDCODE, data will be returned
#     as follows:
#   Word 0 = A956  <== sync pattern
#   Word 1 = A956  <== sync pattern
#   Word 2 = xxxx  <== number of clocks to hold SPI S low
#   Word 3 = yyTT  <== shift SPI IDCODE command in yy position, TT = mfg ID
#   Word 4 = RRSS  <== RR = mem type; SS = mem capacity
#
# See xspi_read_id for an example of reading and displaying the ID information
#=============================================================================
# Constants
#---------------------------------------------------------------
set DEF_XSDB_SPI_CORE     xjtagspi_xsdb.bit
#---------------------------------------------------------------
# Start address of XSDB registers in hex
#---------------------------------------------------------------
set DEF_ADDR_XSDB_REGA 0
set DEF_ADDR_XSDB_REGB 20
set DEF_ADDR_XSDB_REGC 100

set DEF_ADDR_XSPI_CMD_REG 00C1
#---------------------------------------------------------------
# Size of XSDB registers in decimal
# - XSDB register size in 16-bit words
#---------------------------------------------------------------
set DEF_SIZE_WORDS_XSDB_REGA 16
set DEF_SIZE_WORDS_XSDB_REGB 136
set DEF_SIZE_WORDS_XSDB_REGC 536
#---------------------------------------------------------------
set iGlobDebug  0
#---------------------------------------------------------------
set DEF_SYNC_PATTERN   a956

set DEF_CMD_SET_SPICLK_03   A803
#---------------------------------------------------------------
# UltraScale devices have 2 QSPI interfaces:
# - QSPI_0 (goes through STARTUPE3 block)
# - QSPI_1 (goes S, D, and Q pins go through GPIOs)
# - to enable interfaces, set bit 1 (for QSPI_1) or 0 (for QSPI_0)
#   of the 16-bit DEF_CMD_ENABLE_QSPI command to 1; set bits to 0
#   to disable interface
#---------------------------------------------------------------
set DEF_CMD_ENABLE_QSPI     A9C0
#---------------------------------------------------------------
# UltraScale XSDB-SPI core operates on one QSPI core at a time
# - to select QSPI_0, set bit 0 of DEF_CMD_SEL_QSPI to 0
# - to select QSPI_1, set bit 0 of DEF_CMD_SEL_QSPI to 1
#---------------------------------------------------------------
set DEF_CMD_SEL_QSPI        A9D0
#---------------------------------------------------------------

set DEF_ADDR_0_BYTE     0
set DEF_ADDR_3_BYTE     3
set DEF_ADDR_4_BYTE     4

#---------------------------------------
# SPI opcodes/commands in hex
#---------------------------------------
set DEF_SPI_CMD_WRITE_ENABLE 06
set DEF_SPI_CMD_READ         03
set DEF_SPI_CMD_READ_4BYTE_ADDR  13
set DEF_SPI_CMD_READ_STATREG 05
set DEF_SPI_CMD_READ_NONVOLATILE_STATREG B5
set DEF_SPI_CMD_READ_FLAG_STATREG 70
set DEF_SPI_CMD_CLR_FLAG_STATREG  50
set DEF_SPI_CMD_BULK_ERASE   c7
set DEF_SPI_CMD_SECTOR_ERASE d8
set DEF_SPI_CMD_SECTOR_ERASE_4_BYTE dc
set DEF_SPI_CMD_READ_ID      9f
set DEF_SPI_CMD_READ_STATREG1 05
set DEF_SPI_CMD_READ_STATREG2 07
set DEF_SPI_CMD_READ_CFGREG   35
set DEF_SPI_CMD_WRITE_STAT_CFG_REGS   01
set DEF_SPI_CMD_PAGE_PGM     02
#---------------------------------------

set DEF_SIZE_PAGE_PGM_BUFFER 256    ;# Page program buffer size in decimal

set DEF_MAX_ERASE_POLL         100000
set DEF_MAX_SECTOR_ERASE_POLL  4000
set DEF_MAX_PROG_POLL          500

set DEF_WRITE_ENABLE_BIT_MASK  02
set DEF_WRITE_ENABLE_BIT_SET   02
set DEF_ERASE_PROG_MASK        03
set DEF_ERASE_PROG_COMPLETE    00
#-----------------------------------------------------------------------------
proc xspi_setup_hw {} {
  open_hw
  connect_hw_server -host localhost -port 60001
  open_hw_target
  current_hw_device [lindex [get_hw_devices] 0]
}
# end of xspi_setup_hw
#-----------------------------------------------------------------------------
# argByteDataInHex can be NULL
# argByteDataCnt can be > 0 if argByteDataInHex = NULL (used when specifying
# an SPI read command)
#-----------------------------------------------------------------------------
proc makeSpiCmd \
     { argCmdByteInHex \
       argByteAddrCnt \
       argByteAddrInHex \
       argByteDataInHex \
       argByteDataCnt \
       argShiftRegSizeInWords } {

  global iGlobDebug
  global DEF_SYNC_PATTERN

#---------------------------------------------------------------
# Initialize iByteCount to 6 to account for 2 word sync pattern
# and 1 word SPI bit shift count to be added to the beginning of
# the string containing:
# - SPI command
# - SPI address field (if required)
# - SPI data field (if specified)
#---------------------------------------------------------------
  set iByteCount 6
  set varBuf $argCmdByteInHex
  incr iByteCount

#---------------------------------------------------------------
# The byte addr count can be 0 for commands that do not require
# an address field (e.g., write enable)
#---------------------------------------------------------------
  if {$argByteAddrCnt > 0} {
    if {[string length $argByteAddrInHex] != [expr ($argByteAddrCnt * 2)]} {
      puts "Error: addr $argByteAddrInHex must have \
               [format "%d" [expr ($argByteAddrCnt * 2)]] chars"
    }
    append varBuf $argByteAddrInHex
    incr iByteCount $argByteAddrCnt
  }

  set iX [string length $argByteDataInHex]
  if {$iX > 0} {
    if {$iX%2 != 0} {
      puts "Error: byte data $argByteDataInHex must contain an even count of hex chars"
    }
    append varBuf $argByteDataInHex
    incr iByteCount [expr $iX/2]
  }

#---------------------------------------------------------------
# Pad the remainder of the SPI command/data string with the
# pattern "*E"
#---------------------------------------------------------------
  set iY [expr $argShiftRegSizeInWords * 2]
  if {$iY > $iByteCount} {
    set sPadding [format "%0*s" [expr ($iY-$iByteCount)*2] "e"]
  }
  append varBuf $sPadding

# The 1 in the following expression is for the SPI byte command
  set iBitShiftCnt [expr (1 + $argByteAddrCnt \
                        + [expr [string length $argByteDataInHex]/2] ) * 8]
# Add argByteDataCnt if argByteDataInHex == NULL
  if {$argByteDataInHex == ""} {
    incr iBitShiftCnt [expr $argByteDataCnt * 8]
  }

#---------------------------------------------------------------
# Add sync pattern and bit shift count to head of SPI command
# and data string
#---------------------------------------------------------------
  set sTmp $DEF_SYNC_PATTERN$DEF_SYNC_PATTERN
  set sStr [format "%04x" $iBitShiftCnt]
  append sTmp $sStr

  append sTmp $varBuf
  set varBuf $sTmp

# reverse the word order to make it compatible with the Vivado write_hw
# tcl command
  set numchars [string length $varBuf]
  for {set iX 0} {$iX < $argShiftRegSizeInWords} {incr iX} {
    set sWord [string range $varBuf \
             [expr $numchars - (($iX + 1) * 4)    ] \
             [expr $numchars - ( $iX      * 4) - 1]]
    if {$iX == 0} {
      set sSpiCmdData $sWord
    } else {
      append sSpiCmdData $sWord
    }
  }

  if {$iGlobDebug == 1} {
    puts "varBuf BitCnt = \[$iBitShiftCnt\] hex chars = \[[format "%d" [string length $varBuf]]\] = $varBuf"
    puts "sSpiCmdData [format "%d" [string length $sSpiCmdData]] = $sSpiCmdData"
  }

  return $sSpiCmdData
}
# end of makeSpiCmd
#-----------------------------------------------------------------------------
proc displayData {argData argDataWordSize argDisplayMode} {
  global iGlobDebug

  set iY 0
  set numchars [string length $argData]
  if {$iGlobDebug == 1} {
    puts "numchars -> $numchars"
  }
  for {set iX 0} { $iX < $argDataWordSize} {incr iX} {
    set sWord [string range $argData \
             [expr $numchars - (($iX + 1) * 4)    ] \
             [expr $numchars - ( $iX      * 4) - 1]]
    if {$argDisplayMode == 0} {
      puts "$iX = $sWord"
    } else {
      if {$iX >= 5 && $iX < 133} {
        if {($iY + 1)%16 == 0} {
          puts "$sWord"
  	set iY 0
        } else {
          puts -nonewline "$sWord "
  	incr iY
        }
      }
    }
  }
}
# end of displayData
#-----------------------------------------------------------------------------
proc getByteData {argData argDataWordSize argByteIndex} {
  global iGlobDebug

  set iY 0
  set numchars [string length $argData]
  if {$iGlobDebug == 1} {
    puts "numchars -> $numchars"
  }
  if {$argByteIndex > ($argDataWordSize * 2)} {
    puts "Byte index greater than number of bytes in string"
    puts "- data word size = $argDataWordSize"
    puts "- byte index $argByteIndex"
    return "XX"
  }
  set sWord [string range $argData \
           [expr $numchars - (($argByteIndex + 1) * 2)    ] \
           [expr $numchars - ( $argByteIndex      * 2) - 1]]
  return $sWord
}
# end of getByteData
#-----------------------------------------------------------------------------
proc xspi_read_id {} {

  global DEF_ADDR_0_BYTE
  global DEF_ADDR_3_BYTE
  global DEF_SIZE_WORDS_XSDB_REGA
  global DEF_SIZE_WORDS_XSDB_REGB
  global DEF_SIZE_WORDS_XSDB_REGC
  global DEF_SPI_CMD_READ_ID
  global DEF_ADDR_XSDB_REGA
  global DEF_ADDR_XSDB_REGB
  global iGlobDebug

  set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_READ_ID $DEF_ADDR_0_BYTE 0 "" 9 $DEF_SIZE_WORDS_XSDB_REGA]
  write_hw -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA $sSpiCmd
  write_hw -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA $sSpiCmd

  set sDevId [read_hw  -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA]

  displayData $sDevId $DEF_SIZE_WORDS_XSDB_REGA 0
  puts "Manufacturer ID  : [getByteData $sDevId $DEF_SIZE_WORDS_XSDB_REGA 6]"
  puts "Device ID"
  puts "- memory type    : [getByteData $sDevId $DEF_SIZE_WORDS_XSDB_REGA 9]"
  puts "- memory capacity: [getByteData $sDevId $DEF_SIZE_WORDS_XSDB_REGA 8]"
}
# end of xspi_read_id
#-----------------------------------------------------------------------------
# 2014/04/30: Read S25FL127S status registers 1 and 2 and configuration register
# - bit 7 of status reg 2 determines sector size
#   = 1: uniform 256 kB sectors
#     0: 64 kB sectors
#-----------------------------------------------------------------------------
proc xspi_read_statregs {} {

  global DEF_ADDR_0_BYTE
  global DEF_ADDR_3_BYTE
  global DEF_SIZE_WORDS_XSDB_REGA
  global DEF_SIZE_WORDS_XSDB_REGB
  global DEF_SIZE_WORDS_XSDB_REGC
  global DEF_SPI_CMD_READ_ID
  global DEF_SPI_CMD_READ_STATREG1
  global DEF_SPI_CMD_READ_STATREG2
  global DEF_SPI_CMD_READ_CFGREG
  global DEF_ADDR_XSDB_REGA
  global DEF_ADDR_XSDB_REGB
  global iGlobDebug

  set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_READ_STATREG1 $DEF_ADDR_0_BYTE 0 "" 9 $DEF_SIZE_WORDS_XSDB_REGA]
  write_hw -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA $sSpiCmd

  set sDevId [read_hw  -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA]

  puts "Status Register 1:"
#  displayData $sDevId $DEF_SIZE_WORDS_XSDB_REGA 0
  puts "- bits \[7:0\] : [getByteData $sDevId $DEF_SIZE_WORDS_XSDB_REGA 6]"

  set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_READ_STATREG2 $DEF_ADDR_0_BYTE 0 "" 9 $DEF_SIZE_WORDS_XSDB_REGA]
  write_hw -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA $sSpiCmd

  set sDevId [read_hw  -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA]

  puts "Status Register 2:"
#  displayData $sDevId $DEF_SIZE_WORDS_XSDB_REGA 0
  puts "- bits \[7:0\] : [getByteData $sDevId $DEF_SIZE_WORDS_XSDB_REGA 6]"

  set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_READ_CFGREG $DEF_ADDR_0_BYTE 0 "" 9 $DEF_SIZE_WORDS_XSDB_REGA]
  write_hw -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA $sSpiCmd

  set sDevId [read_hw  -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA]

  puts "Configuration Register :"
#  displayData $sDevId $DEF_SIZE_WORDS_XSDB_REGA 0
  puts "- bits \[7:0\] : [getByteData $sDevId $DEF_SIZE_WORDS_XSDB_REGA 6]"
}
# end of xspi_read_statregs
#-----------------------------------------------------------------------------
# 2014/04/30: Program S25FL127S status registers 1 and 2 and configuration register
# - bit 7 of status reg 2 s OTP and determines sector size
#   = 1: uniform 256 kB sectors
#     0: 64 kB sectors
# - write register command writes STATREG1, CFG REG, STATREG2 in one SPI command
#-----------------------------------------------------------------------------
proc xspi_prog_statregs {} {

  global DEF_ADDR_0_BYTE
  global DEF_ADDR_3_BYTE
  global DEF_SIZE_WORDS_XSDB_REGA
  global DEF_SIZE_WORDS_XSDB_REGB
  global DEF_SIZE_WORDS_XSDB_REGC
  global DEF_SPI_CMD_READ_ID
  global DEF_SPI_CMD_READ_STATREG1
  global DEF_SPI_CMD_READ_STATREG2
  global DEF_SPI_CMD_READ_CFGREG
  global DEF_SPI_CMD_WRITE_STAT_CFG_REGS
  global DEF_ADDR_XSDB_REGA
  global DEF_ADDR_XSDB_REGB
  global DEF_ERASE_PROG_MASK
  global DEF_ERASE_PROG_COMPLETE
  global DEF_MAX_PROG_POLL
  global iGlobDebug

 if {!([xspi_write_enable])} {
   return 0
 }

# Set to 1 for S25FL127S
  if {0} {
  set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_WRITE_STAT_CFG_REGS $DEF_ADDR_0_BYTE 0 "020080" 0 $DEF_SIZE_WORDS_XSDB_REGA]
  } else {
#
  set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_WRITE_STAT_CFG_REGS $DEF_ADDR_0_BYTE 0 "000000" 0 $DEF_SIZE_WORDS_XSDB_REGA]
  }
  puts $sSpiCmd
  write_hw -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA $sSpiCmd

  set iRetVal [checkValue $DEF_ERASE_PROG_MASK $DEF_ERASE_PROG_COMPLETE $DEF_MAX_PROG_POLL 8 \
"Possible prog error; max prog polling iteration reached" 0]
}
# end of xspi_prog_statregs
#-----------------------------------------------------------------------------
# 2014/05/30: Program S25FL128S and S25FL256S status register and configuration register
# - specifically developed to program quad enable bit in config reg bit 1
# - write register command writes STATREG, CFG REG in one SPI command
#   = compare against xspi_prog_statregs
# If argMode = 1: set quad enable bit, else clear quad enable bit
#-----------------------------------------------------------------------------
proc xspi_prog_quad_enable { argMode } {

  global DEF_ADDR_0_BYTE
  global DEF_ADDR_3_BYTE
  global DEF_SIZE_WORDS_XSDB_REGA
  global DEF_SIZE_WORDS_XSDB_REGB
  global DEF_SIZE_WORDS_XSDB_REGC
  global DEF_SPI_CMD_READ_ID
  global DEF_SPI_CMD_READ_STATREG1
  global DEF_SPI_CMD_READ_STATREG2
  global DEF_SPI_CMD_READ_CFGREG
  global DEF_SPI_CMD_WRITE_STAT_CFG_REGS
  global DEF_ADDR_XSDB_REGA
  global DEF_ADDR_XSDB_REGB
  global DEF_ERASE_PROG_MASK
  global DEF_ERASE_PROG_COMPLETE
  global DEF_MAX_PROG_POLL
  global iGlobDebug

 if {!([xspi_write_enable])} {
   return 0
 }

  if {$argMode == 1} {
    set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_WRITE_STAT_CFG_REGS $DEF_ADDR_0_BYTE 0 "0202" 0 $DEF_SIZE_WORDS_XSDB_REGA]
  } else {
    set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_WRITE_STAT_CFG_REGS $DEF_ADDR_0_BYTE 0 "0200" 0 $DEF_SIZE_WORDS_XSDB_REGA]
  }
  puts $sSpiCmd
  write_hw -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA $sSpiCmd

  set iRetVal [checkValue $DEF_ERASE_PROG_MASK $DEF_ERASE_PROG_COMPLETE $DEF_MAX_PROG_POLL 8 \
"Possible prog error; max prog polling iteration reached" 0]
}
# end of xspi_prog_quad_enable
#-----------------------------------------------------------------------------
proc xspi_clear_flag_status_reg {} {
  global DEF_ADDR_XSDB_REGA
  global DEF_ADDR_XSDB_REGB
  global DEF_SIZE_WORDS_XSDB_REGA
  global DEF_SIZE_WORDS_XSDB_REGB
  global DEF_SPI_CMD_CLR_FLAG_STATREG
  global DEF_ADDR_0_BYTE
  global DEF_WRITE_ENABLE_BIT_MASK
  global DEF_WRITE_ENABLE_BIT_SET

# Send clear flag status command
  set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_CLR_FLAG_STATREG $DEF_ADDR_0_BYTE 0 "" 0 $DEF_SIZE_WORDS_XSDB_REGB]
  write_hw -burst 1 0 $DEF_ADDR_XSDB_REGB $DEF_SIZE_WORDS_XSDB_REGB \
      $sSpiCmd

}
# end of xspi_clear_flag_status_reg
#-----------------------------------------------------------------------------
proc xspi_write_enable {} {
  global DEF_ADDR_XSDB_REGA
  global DEF_ADDR_XSDB_REGB
  global DEF_SIZE_WORDS_XSDB_REGA
  global DEF_SIZE_WORDS_XSDB_REGB
  global DEF_SPI_CMD_WRITE_ENABLE
  global DEF_ADDR_0_BYTE
  global DEF_WRITE_ENABLE_BIT_MASK
  global DEF_WRITE_ENABLE_BIT_SET


# Send write enable command
  set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_WRITE_ENABLE $DEF_ADDR_0_BYTE 0 "" 0 $DEF_SIZE_WORDS_XSDB_REGB]
  write_hw -burst 1 0 $DEF_ADDR_XSDB_REGB $DEF_SIZE_WORDS_XSDB_REGB \
      $sSpiCmd

  return [checkValue $DEF_WRITE_ENABLE_BIT_MASK $DEF_WRITE_ENABLE_BIT_SET 1 1\
          "write enable bit not set" 0]
}
# end of xspi_write_enable
#-----------------------------------------------------------------------------
proc checkValue {argMask argExpectedWord argMaxCheckIteration \
                 argIterInterval argMsg argShowPollStatus} {
  global iGlobDebug

  for {set iX 0} {$iX < $argMaxCheckIteration} {incr iX} {
    set iStatusByte 0x[xspi_read_status_reg]
    if {($iStatusByte & $argMask) == $argExpectedWord} {
      if {$iGlobDebug == 1 || $argShowPollStatus == 1} {
        puts "=> Read expected status reg value \[$argExpectedWord\] at \
iteration \[$iX of $argMaxCheckIteration\]"
      }
      break;
    }
    if {$iX%$argIterInterval == 1} {
      if {$iGlobDebug == 1 || $argShowPollStatus == 1} {
        puts "=> Polling SPI status reg: read \[$iStatusByte\] expecting \
\[$argExpectedWord\] poll iteration \[$iX of $argMaxCheckIteration\]"
      }
    }
  } ;# end of FOR iX < argMaxCheckIteration
  if {$iX < $argMaxCheckIteration} {
    set iStatusByte [xspi_read_flag_status_reg]
if {0} {
    if {$iStatusByte != "80"} {
      puts "=============== Flag status reg = $iStatusByte ==========="
      xspi_clear_flag_status_reg
    }
}
    return 1
  }
  puts "Error: $argMsg"
  return 0
}
# end of checkValue
#-----------------------------------------------------------------------------
proc xspi_read_status_reg {} {
  global DEF_ADDR_0_BYTE
  global DEF_SPI_CMD_READ_STATREG
  global DEF_ADDR_XSDB_REGA
  global DEF_ADDR_XSDB_REGB
  global DEF_SIZE_WORDS_XSDB_REGA
  global DEF_SIZE_WORDS_XSDB_REGB
  global iGlobDebug

  set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_READ_STATREG $DEF_ADDR_0_BYTE 0 "" 4 $DEF_SIZE_WORDS_XSDB_REGA]
  write_hw -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA $sSpiCmd
  set sStatusVal [read_hw  -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA]

  set numchars [string length $sStatusVal]
  if {$iGlobDebug == 1} {
    puts "numchars -> $numchars"
    for {set iX 0} { $iX < $DEF_SIZE_WORDS_XSDB_REGB} {incr iX} {
      set sWord [string range $sStatusVal \
               [expr $numchars - (($iX + 1) * 4)    ] \
               [expr $numchars - ( $iX      * 4) - 1]]
      puts "$iX = $sWord"
    }
  }
  set iX 6
  set sByte [string range $sStatusVal \
           [expr $numchars - (($iX + 1) * 2)    ] \
           [expr $numchars - ( $iX      * 2) - 1]]
  if {$iGlobDebug == 1} {
    puts "Status reg = $sByte"
  }
  return $sByte
}
# end of xspi_read_status_reg
#-----------------------------------------------------------------------------
proc xspi_read_nonvolatile_status_reg {} {
  global DEF_ADDR_0_BYTE
  global DEF_SPI_CMD_READ_STATREG
  global DEF_SPI_CMD_READ_NONVOLATILE_STATREG
  global DEF_ADDR_XSDB_REGA
  global DEF_ADDR_XSDB_REGB
  global DEF_SIZE_WORDS_XSDB_REGA
  global DEF_SIZE_WORDS_XSDB_REGB
  global iGlobDebug

  set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_READ_NONVOLATILE_STATREG \
                          $DEF_ADDR_0_BYTE 0 "" 4 $DEF_SIZE_WORDS_XSDB_REGA]
  write_hw -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA $sSpiCmd
  set sStatusVal [read_hw  -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA]

  set numchars [string length $sStatusVal]
  if {$iGlobDebug == 1} {
    puts "numchars -> $numchars"
    for {set iX 0} { $iX < $DEF_SIZE_WORDS_XSDB_REGA} {incr iX} {
      set sWord [string range $sStatusVal \
               [expr $numchars - (($iX + 1) * 4)    ] \
               [expr $numchars - ( $iX      * 4) - 1]]
      puts "$iX = $sWord"
    }
  }
  set iX 6
  set sByte0 [string range $sStatusVal \
           [expr $numchars - (($iX + 1) * 2)    ] \
           [expr $numchars - ( $iX      * 2) - 1]]
  set iX 9
  set sByte1 [string range $sStatusVal \
           [expr $numchars - (($iX + 1) * 2)    ] \
           [expr $numchars - ( $iX      * 2) - 1]]
  puts "Non-volatile status reg \[15:0\] = $sByte0$sByte1"
  return $sByte0$sByte1
}
# end of xspi_read_nonvolatile_status_reg
#-----------------------------------------------------------------------------
proc xspi_read_flag_status_reg {} {
  global DEF_ADDR_0_BYTE
  global DEF_SPI_CMD_READ_FLAG_STATREG
  global DEF_ADDR_XSDB_REGA
  global DEF_ADDR_XSDB_REGB
  global DEF_SIZE_WORDS_XSDB_REGA
  global DEF_SIZE_WORDS_XSDB_REGB
  global iGlobDebug

  set sSpiCmd [makeSpiCmd $DEF_SPI_CMD_READ_FLAG_STATREG \
$DEF_ADDR_0_BYTE 0 "" 4 $DEF_SIZE_WORDS_XSDB_REGA]

  write_hw -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA $sSpiCmd
  set sStatusVal [read_hw  -burst 1 0 $DEF_ADDR_XSDB_REGA $DEF_SIZE_WORDS_XSDB_REGA]

  set numchars [string length $sStatusVal]
  if {$iGlobDebug == 1} {
    puts "numchars -> $numchars"
      for {set iX 0} { $iX < $DEF_SIZE_WORDS_XSDB_REGB} {incr iX} {
      set sWord [string range $sStatusVal \
               [expr $numchars - (($iX + 1) * 4)    ] \
               [expr $numchars - ( $iX      * 4) - 1]]
      puts "$iX = $sWord"
    }
  }
  set iX 6
  set sByte [string range $sStatusVal \
           [expr $numchars - (($iX + 1) * 2)    ] \
           [expr $numchars - ( $iX      * 2) - 1]]
  if {$iGlobDebug == 1} {
    puts "Flag status reg = $sByte"
  }
  return $sByte
}
# end of xspi_read_flag_status_reg
#-----------------------------------------------------------------------------
proc xspi_set_spi_clk {} {
  global DEF_ADDR_XSPI_CMD_REG
  global DEF_CMD_SET_SPICLK_03

  write_hw 0 $DEF_ADDR_XSPI_CMD_REG 1 $DEF_CMD_SET_SPICLK_03
}
# end of XSPI_SET_SPI_CLK
#-----------------------------------------------------------------------------
proc xspi_get_xsdb_core {} {
  get_hw_cores
  current_hw_core [lindex [get_hw_cores] 0]
}
# end of xspi_get_xsdb_core
#-----------------------------------------------------------------------------
proc xspi_close_hw {} {
  disconnect_hw_server localhost
}
# end of xspi_close_hw
#-----------------------------------------------------------------------------
#=============================================================================
# end of tcl test file
#=============================================================================
