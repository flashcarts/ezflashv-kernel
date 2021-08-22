
static void boot_GBAMP(void)
{
	REG_IME = IME_DISABLE;	// Disable interrupts
	REG_IF = REG_IF;	// Acknowledge interrupt
	*((vu32*)0x027FFE34) = (u32)0x08000000;	// Bootloader start address
	swiSoftReset();	// Jump to boot loader
}
