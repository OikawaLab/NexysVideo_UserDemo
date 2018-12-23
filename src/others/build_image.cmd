@echo off

set OUTFILE=.\nexys_video_demo
set MMI=%~dp0..\..\sdk\system_hw_platform_0\system_wrapper.mmi
set BIT=%~dp0..\..\sdk\system_hw_platform_0\system_wrapper.bit
set ELF=%~dp0..\..\sdk\bootloader\Release\bootloader.elf
set SREC=%~dp0..\..\sdk\demo\Release\demo.elf.srec
set DBIT=.\download.bit
set MCS_SCRIPT=%~dp0create_flash_mcs.tcl

set ERRORLEVEL=

echo *** building download.bit ***

if not exist %MMI% GOTO notfound
if not exist %BIT% GOTO notfound
if not exist %ELF% GOTO notfound

call updatemem -force -meminfo "%MMI%" -bit "%BIT%" -data "%ELF%" -proc system_i/microblaze_0 -out %DBIT%
if %ERRORLEVEL% NEQ 0 goto updatememerr

call vivado -mode tcl -source "%MCS_SCRIPT%" -tclargs -b%DBIT:\=/% -s%SREC:\=/% -o%OUTFILE:\=/%.mcs
if %ERRORLEVEL% NEQ 0 goto fail

goto success

:notfound
echo Some required files were not found during the build process.
echo Files required in this step are:
echo %MMI%
echo %BIt%
echo %ELF%
goto fail

:updatememerr
:fail
echo *** building download.bit failed ***
goto end

:success
echo *** building download.bit: done ***

:end
