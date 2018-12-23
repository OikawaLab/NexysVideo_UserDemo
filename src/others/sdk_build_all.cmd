@echo off
set WS=%~dp0..\..\sdk
set PRJ=%~dp0..\..\sdk
set HW0=%PRJ%\system_hw_platform_0
set BSP0=%PRJ%\bist_bsp
set APP0=%PRJ%\bootloader
set APP1=%PRJ%\demo
set APP2=%PRJ%\bist
set errorlevel=

echo *** setting up SDK environment ***
call setupEnv.bat
if not defined RDI_APPROOT (
   echo *** setup error %errorlevel% ***
   exit /b %errorlevel%
)
set XILINX_SDK_PATH=%RDI_APPROOT%
echo *** setup step: done ***

echo *** importing projects into workspace ***
%XILINX_SDK_PATH%\eclipse\win64.o\eclipsec --launcher.suppressErrors -vm %XILINX_SDK_PATH%/tps/win64/jre/bin -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -import "%HW0%" -import "%BSP0%" -import "%APP0%" -import "%APP1%" -import "%APP2%" -data %WS% -vmargs -Dorg.eclipse.cdt.core.console=org.eclipse.cdt.core.systemConsole
rem call xsdk -wait -workspace "%WS%" -eclipseargs --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -import "%HW0%" -import "%BSP0%" -import "%APP0%" -import "%APP1%" -vmargs -Dorg.eclipse.cdt.core.console=org.eclipse.cdt.core.systemConsole
if %errorlevel% NEQ 0 (
   echo *** import error %errorlevel% ***
   exit /b %errorlevel%
)
echo *** import step: done ***

echo *** starting build step ***
rem %XILINX_SDK_PATH%\eclipse\win64.o\eclipsec --launcher.suppressErrors -vm %XILINX_SDK_PATH%/tps/win64/jre/bin -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -cleanBuild all -data %WS% -vmargs -Dorg.eclipse.cdt.core.console=org.eclipse.cdt.core.systemConsole
call xsdk -wait -workspace "%WS%" -eclipseargs --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -cleanBuild all -vmargs -Dorg.eclipse.cdt.core.console=org.eclipse.cdt.core.systemConsole
if %errorlevel% NEQ 0 (
   echo *** build error %errorlevel% ***
   exit /b %errorlevel%
)

echo *** build step: done ***
exit /b 0