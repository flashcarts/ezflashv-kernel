#include <NDS.h>
#include "ez5_language.h"

//全局变量
u8 gl_language=0;
u8 gl_speedAuto=0;
u16 gl_speed=1000;
char pSaveFileName[256];
u8 gl_SaveType=2;
u8 *gl_pSaveTypeBuf=0;
int  gl_SizeTypeFileSize=0;
u32 gl_SaveTypeOff=0;

//字符串
char* gl_speedset;
char* gl_speedAutoDetect;
char* gl_strSaveType;
char* gl_strAboutMsg;//5--Loader Flash地址线检测
char* gl_strAMsg;
char* gl_strBMsg;
char* gl_strLRMsg;
char* gl_strLXMsg;
char* gl_strRXMsg;
char* gl_strQuit;
char* gl_strSaveQuit;
char* gl_strBaseMoonShell;
char* gl_strThanksMoonShell;


char* gl_strLoadSave;
char* gl_strBackSave;
char* gl_strLoadGame;
//中文
const char zh_speedset[]="MicroSD卡速度";
const char zh_speedAutoDetect[]="MicroSD卡速度自动检测";
const char zh_strSaveType[]="存档类型";
const char zh_strAboutMsg[]="帮助";
const char zh_strAMsg[]="A: 执行文件";
const char zh_strBMsg[]="B: 取消/回上一级目录";
const char zh_strLRMsg[]="L+R: 系统设置";
const char zh_strLXMsg[]="L+X/L+Y: 调节存档类型";
const char zh_strRXMsg[]="切换存档档位";
const char zh_strQuit[]="[A]退出";
const char zh_strSaveQuit[]="[B]存盘退出";
const char zh_strBaseMoonShell[]="本软件是基于moonshell修改而成";
const char zh_strThanksMoonShell[]="在此感谢作者moonlight ";

const char zh_strLoadSave[]="向存档芯片中写入存档";
const char zh_strBackSave[]="备份存档到MicroSD";
const char zh_strLoadGame[]="初始化...";
//英文
const  char en_speedset[]="MicroSD Speed set";
const  char en_speedAutoDetect[]="MicroSD Speed Auto Detect";
const  char en_strSaveType[]="Save Type";
const  char en_strAboutMsg[]="Help";
const char en_strAMsg[]="A: Launch File";
const char en_strBMsg[]="B: Cancel/Go Prev Folder";
const char en_strLRMsg[]="L+R: Setting ";
const char en_strLXMsg[]="L+X/L+Y: Change Save Setting";
const char en_strRXMsg[]="Toggle Saver";
const char en_strQuit[]="[A]Quit";
const char en_strSaveQuit[]="[B]Save and Quit";
const char en_strBaseMoonshell[]="This software besed on MoonShell";
const char en_strThanksMoonShell[]="many thanks to moonlight ";

const char en_strLoadSave[]="Write save to savechip";
const char en_strBackSave[]="Backup save to MicroSD";
const char en_strLoadGame[]="Initializing...";

void LoadLanguage()
{
	if(gl_language==1)
  		LoadChinese();
  	else if(gl_language==2)
  		LoadEnglish();
  	else
  	{
  		gl_language==1;
  		LoadChinese();
  	}	
}
void LoadChinese()
{
	gl_speedset=(char *)zh_speedset;
	gl_speedAutoDetect=(char *)zh_speedAutoDetect;
	gl_strSaveType=(char *)zh_strSaveType;
	gl_strAboutMsg=(char*)zh_strAboutMsg;
	gl_strAMsg =(char*)zh_strAMsg;
	gl_strBMsg=(char*)zh_strBMsg;
	gl_strLRMsg=(char*)zh_strLRMsg;
	gl_strLXMsg=(char*)zh_strLXMsg;
	gl_strRXMsg=(char*)zh_strRXMsg;
	gl_strQuit=(char*)zh_strQuit;
	gl_strSaveQuit=(char*)zh_strSaveQuit;
	gl_strBaseMoonShell=(char*)zh_strBaseMoonShell;
	gl_strThanksMoonShell = (char*)zh_strThanksMoonShell;
	gl_strLoadSave=(char *)zh_strLoadSave;
	gl_strBackSave=(char *)zh_strBackSave;
	gl_strLoadGame=(char *)zh_strLoadGame;
}

void LoadEnglish()
{
	gl_speedset=(char *)en_speedset;
	gl_speedAutoDetect=(char *)en_speedAutoDetect;
	gl_strSaveType=(char *)en_strSaveType;
	gl_strAboutMsg=(char*)en_strAboutMsg;
	gl_strAMsg =(char*)en_strAMsg;
	gl_strBMsg=(char*)en_strBMsg;
	gl_strLRMsg=(char*)en_strLRMsg;
	gl_strLXMsg=(char*)en_strLXMsg;
	gl_strRXMsg=(char*)en_strRXMsg;
	gl_strQuit=(char*)en_strQuit;
	gl_strSaveQuit=(char*)en_strSaveQuit;
	gl_strBaseMoonShell=(char*)en_strBaseMoonshell;
	gl_strThanksMoonShell=(char*)en_strThanksMoonShell;
	gl_strLoadSave=(char *)en_strLoadSave;
	gl_strBackSave=(char *)en_strBackSave;
	gl_strLoadGame=(char *)en_strLoadGame;
}

s32 GetSaveSize()
{
	switch(gl_SaveType)
	{
		case 0:
			return 0x200;
			break;
		case 1:
			return 0x2000;
			break;
		case 2:
			return 0x4000;
			break;
		case 3:
			return 0x8000;
			break;
		case 4:
			return 0x10000;
			break;
		case 5:
			return 0x40000;
			break;
		case 6:
			return 0x80000;
			break;
		case 0xFF:
			return 0;
			break;
		case 0xFE:
			return 0x2000;
			break;
		default:
			return 0;
			break;
	}	
}