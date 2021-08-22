@echo off

echo クリーンコンパイルのために一時ファイルを削除します。makeの自動実行はしません。
echo 中止する場合は(Windows)閉じるボタンを押すか、(DOS)Ctrl+Cを押して下さい。
pause

rd /q /s arm7\build\
rd /q /s arm9\build\

del arm7\arm7.bin
del arm7\arm7.elf
del arm7\arm7.map
del arm7\arm7.mapsort
del arm9\arm9.bin
del arm9\arm9.elf
del arm9\arm9.map
del arm9\arm9.mapsort

