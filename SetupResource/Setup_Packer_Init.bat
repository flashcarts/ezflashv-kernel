del "Setup.exe"

del "Setup_resource.dat"
del "Setup_resource.res"
rem copy "D:\MyDocuments\NDS\MoonShell\_BOOT_MP.nds" "D:\MyDocuments\NDS\MoonShell\SetupData\"
copy "D:\MyDocuments\NDS\MoonShell\CreateNDSROM_for_EXFS.bat" "D:\MyDocuments\NDS\MoonShell\SetupData\"
md "D:\MyDocuments\NDS\MoonShell\SetupData\wintools"
copy "D:\MyDocuments\NDS\MoonShell\wintools\*.*" "D:\MyDocuments\NDS\MoonShell\SetupData\wintools"
del "D:\MyDocuments\NDS\MoonShell\SetupData\wintools\IMFS_CreateBin.log"

pause
Setup_Packer.exe
