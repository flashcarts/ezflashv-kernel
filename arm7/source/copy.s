.arm
.align 4

.global bxr0
bxr0: @r0 Ìø×ªµØÖ·
	bx  r0
loop:
ldmia r1!,{r4-r11}
stmia r2!,{r4-r11}
subs  r3,r3,#0x20
bhi		loop
mov r0,#0x2700000
add r0,r0,#0xFF000
ldr	r1,[r0,#0xe24]
str r1,[r0,#0xFC8]
swi 0

.align 4
.global CopyAndBoot @r0 source r1 dest r2 size r3 jmpto
CopyAndBoot:
ldr r4,=0x027FFFC8
ldr	r5,=0x027FFE24
ldr	r6,[r5]
str	r6,[r4]
swi 0x0

.pool
.align 4

.end
