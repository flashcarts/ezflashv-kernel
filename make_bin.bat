@echo off
call setenv_devkitPror17.bat

:loop
cls
goto skipclean
del /Q arm7\build\*.*
del /Q arm9\build\*.*
:skipclean

del arm7\arm7.bin
del arm7\arm7.elf
del arm7\arm7.map
del arm9\arm9.bin
del arm9\arm9.elf
del arm9\arm9.map
del arm9\arm9_sort.map
del arm9\arm9_objdump_*.txt
del _BOOT_MP.nds
rem del \\m700\mydoc\rt2560_app_wmb_1_4_beta_1\data\ndstemp.nds
make
if exist _BOOT_MP.nds goto run
pause
goto loop

:run
rem C:\ndsdev\emulators\dualis\dualis.exe _BOOT_MP.nds
pause
goto loop

