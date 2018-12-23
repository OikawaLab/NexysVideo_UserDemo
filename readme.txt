----------------------------
-- Hardware
----------------------------
To re-create a Vivado project:
0. Make sure the directory does not already contain a project with the same name. 
   You may run cleanup.cmd to delete everything except the utility files.
1. Open either the Vivado Tcl shell or the Tcl Window in Vivado GUI 
2. cd to the directory where you want the project created. 
   For example: <repo>/proj
3. run: source ./create_project.tcl

To make sure changes to the project are checked into git:
	Export hardware with bitstream to ./hw_handoff/.
	Export block design to ./src/bd/, if there is one.
	If there are changes to the Vivado project settings, go to File -> 
    Write Project TCL. Copy relevant TCL commands to proj/create_project.tcl. 
    This is the only project-relevant file checked into Git.
	Store all the project sources in src/. Design files go into src/hdl/, 
    constraints into src/constraints.
	Any IPs instantiated OUTSIDE BLOCK DESIGNS need to be created in /src/ip. 
    Use the IP Location button in the IP customization wizard to specify a 
    target directory. Only *.xci and *.prj files are checked in from here.
	If using MIG outside block designs, manually move the MIG IP to the 
    src/ip folder.

----------------------------
-- Software
----------------------------
Workspace folder: ./sdk
The workspace folder is versioned on Git without workspace information. This means
that when first cloning the repository and opening the ./sdk folder as workspace, it
will be empty in SDK. The workspace needs to be re-built locally by manually importing projects, BSPs and
hardware platforms. Once this is done locally the first time, subsequent git pulls will not
touch the workspace. New imports will only be necessary when new projects appear.
Use File -> Import -> Existing projects into Workspace and select ./sdk as root directory. Check the
projects you want imported and make sure "Copy projects into workspace" is unchecked.
"Internal Error" during BSP import can be ignored. Just re-generate BSPs.

"demo" project specific:
   This application is loaded from the QSPI flash by the bootloader. Upon warm-boot (reset with CPU_RESETN) the
   processor jumps to the reset vector. If the demo application is compiled normally, it populates the reset vector.     This means that the processor is reset to the demo, not to the bootloader. Unfortunately the memory sections for
   demo are not re-initialized and contain the previous run's values. This will result in driver init failures and
   heap allocation errors. To fix this the reset vectors needs to point to the bootloader in BRAM. This requires
   the demo to be compiled with specific settings. See my forum post for solution:
http://forums.xilinx.com/t5/Embedded-Processor-System-Design/proper-init-of-bootloaded-code-after-warm-reset/td-p/559237
   The compiler option will result in a "demo" code which is only meant to be run with the bootloader. Only the 
   Release configuration has this set in project properties. The Debug configuration generates an elf that can be
   run without the bootloader. Use the Debug build during development and the Release build for, well... release.
   loading "demo" over JTAG, during development.
