
.arm
.align 4

.global DS_Read
.global dsCardi_ReadCard

.global bxr0
bxr0: @r0 跳转地址
  mov r12,r0
  mov r0,r1
  mov r1,r2
  bx  r12
	
DS_Read: @r0 address, r1 buffer //读512
	mov r8,r0
	mov r9,r1
	mov r11,#0x04000000
	ldr r10,[r11,#0x204]
	bic	r10,r10,#0x800
	str	r10,[r11,#0x204]
	mov r1,r0
	MOV R0, R1,LSR#8
	ORR R0, R0, #0xB7000000
	MOV R1, R1,LSL#24
	mov	r12,lr 
	bl  SetRomOP
	mov lr,r12
	ldr r10,=0x00587F00 @flag
	BIC R10, R10, #0x7000000
	ORR R10, R10, #0xA1000000
	ldr r11,=0x040001a4
	ldr r12,=0x04100010
	str	r10,[r11]
loop1:	
	ldr r10,[r11]
	ands r1,r10,#0x800000
	beq loop2
	ldr r0,[r12]
	str	r0,[r9],#+4
loop2	:
	ands r1,r10,#0x80000000
	bne  loop1
	mov  r0,r8
	mov  r1,r9
	bx  lr	
dsCardi_ReadCard: @r0 address, r1 buffer //读512
	stmfd sp!,{r0-r12,lr}
	mov r8,r0
	mov r9,r1
	mov r11,#0x04000000
	ldr r10,[r11,#0x204]
	bic	r10,r10,#0x800
	str	r10,[r11,#0x204]
	mov r1,r0
	MOV R0, R1,LSR#8
	ORR R0, R0, #0xB7000000
	MOV R1, R1,LSL#24
	mov	r12,lr
	bl  SetRomOP
	mov lr,r12
	ldr r10,=0x00586000 @flag
	BIC R10, R10, #0x7000000
	ORR R10, R10, #0xA1000000
	ldr r11,=0x040001a4
	ldr r12,=0x04100010
	str	r10,[r11]
dsloop1:	
	ldr r10,[r11]
	ands r1,r10,#0x800000
	beq dsloop2
	ldr r0,[r12]
	str	r0,[r9],#+4
dsloop2	:
	ands r1,r10,#0x80000000
	bne  dsloop1
	ldmfd sp!,{r0-r12,lr}
	bx  lr	
.pool
SetRomOP: 	@r0,r1 传送的参数
	ldr r11,=0x040001A4
oploop:
	ldr r10,[r11]
	ands r10,r10,#0x80000000
	bne oploop
	mov r10,#0xc0
	ldr r11,=0x040001A1
	strb r10,[r11]
	mov	 r10,r0,lsr#24
	ldr	 r11,=0x040001A8
	strb r10,[r11],#+1
	mov	 r10,r0,lsr#16
	strb r10,[r11],#+1
	mov	 r10,r0,lsr#8
	strb r10,[r11],#+1
	strb r0,[r11],#+1
	
	mov	 r10,r1,lsr#24
	strb r10,[r11],#+1
	mov	 r10,r1,lsr#16
	strb r10,[r11],#+1
	mov	 r10,r1,lsr#8
	strb r10,[r11],#+1
	strb r1,[r11],#+1
	bx  lr


.pool
.align 4

.end
