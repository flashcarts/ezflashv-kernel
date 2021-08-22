
#include <NDS.h>

extern u8 PM_GetRegister(int reg);
extern void PM_SetRegister(int reg, int control);

//void a7sleep_dummy(void);
//void a7sleep(void);
//void a7lcdbl(int sw);
void a7lcd_select(int control);
void a7led(int sw);
//void a7led_simple(bool onflag);
void a7poff(void);

void a7SetSoundAmplifier(bool e);

/* ---------------------------------------------------------
arm7/main.cpp add

#include "a7sleep.h"

//	XKEYS��IPC�ɏ�������ł��邠����ɂł������

    u32 xkeys=XKEYS;
    IPC->buttons = xkeys;

    if(xkeys == 0x00FF)	//	�p�l���N���[�Y
    {
//	�p�l���N���[�Y��ԂŌĂԎ�
//	�p�l���I�[�v���ŕ��A���܂�
		a7sleep();
    }



//	LCD�o�b�N���C�gOFF/ON����

    u32 xkeys=XKEYS;
    IPC->buttons = xkeys;

    if(xkeys == 0x00FF)	//	�p�l���N���[�Y
    {
		a7lcdbl(0);
    }
    else
    {
		a7lcdbl(1);
    }


-----------------------------------------------------------*/

