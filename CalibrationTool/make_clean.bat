@echo off

echo �N���[���R���p�C���̂��߂Ɉꎞ�t�@�C�����폜���܂��Bmake�̎������s�͂��܂���B
echo ���~����ꍇ��(Windows)����{�^�����������A(DOS)Ctrl+C�������ĉ������B
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

