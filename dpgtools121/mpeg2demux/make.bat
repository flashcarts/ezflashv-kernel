call D:\MyDocuments\bcctest\set_bccenv.bat
@echo off
cls
del extract_mpeg2.exe
bcc32.exe -O2 -DHAVE_CONFIG_H *.c
del *.obj
pause
make.bat
