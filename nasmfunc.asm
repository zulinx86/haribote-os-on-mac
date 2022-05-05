; nasmfunc

bits 32

	global io_hlt, io_cli, io_sti, io_stihlt
	global io_in8, io_in16, io_in32
	global io_out8, io_out16, io_out32
	global io_load_eflags, io_store_eflags
	global load_cr0, store_cr0
	global load_gdtr, load_idtr, load_tr
	global asm_inthandler20, asm_inthandler21, asm_inthandler2c
	extern inthandler20, inthandler21, inthandler2c
	global memtest_sub
	global taskswitch3, taskswitch4

section .text

io_hlt:				; void io_hlt(void);
	hlt
	ret

io_cli:				; void io_cli(void);
	cli
	ret

io_sti:				; void io_sti(void);
	sti
	ret

io_stihlt:			; void io_stihlt(void);
	sti
	hlt
	ret

io_in8:				; int io_in8(int port);
	mov edx,[esp+4]		; port
	mov eax,0
	in al,dx
	ret

io_in16:			; int io_in16(int port);
	mov edx,[esp+4]
	mov eax,0
	in ax,dx
	ret

io_in32:			; int io_in32(int port);
	mov edx,[esp+4]
	in eax,dx
	ret

io_out8:			; void io_out8(int port, int data);
	mov edx,[esp+4]
	mov eax,[esp+8]
	out dx,al
	ret

io_out16:			; void io_out16(int port, int data);
	mov edx,[esp+4]
	mov eax,[esp+8]
	out dx,ax
	ret

io_out32:			; void io_out32(int port, int data);
	mov edx,[esp+4]
	mov eax,[esp+8]
	out dx,eax
	ret

io_load_eflags:		; int io_load_eflags(void);
	pushfd
	pop eax
	ret

io_store_eflags:	; void io_store_eflags(int eflags);
	mov eax,[esp+4]
	push eax
	popfd
	ret

load_cr0:			; int load_cr0(void);
	mov eax,cr0
	ret

store_cr0:			; void store_cr0(int cr0);
	mov eax,[esp+4]
	mov cr0,eax
	ret

load_gdtr:			; void load_gdtr(int limit, int addr);
	mov ax,[esp+4]
	mov [esp+6],ax
	lgdt [esp+6]
	ret

load_idtr:			; void load_idtr(int limit, int addr);
	mov ax,[esp+4]
	mov [esp+6],ax
	lidt [esp+6]
	ret

load_tr:			; void load_tr(int tr);
	ltr [esp+4]
	ret

asm_inthandler20:
	push es
	push ds
	pushad
	mov eax,esp
	push eax
	mov ax,ss
	mov ds,ax
	mov es,ax
	call inthandler20
	pop eax
	popad
	pop ds
	pop es
	iretd

asm_inthandler21:
	push es
	push ds
	pushad
	mov eax,esp
	push eax
	mov ax,ss
	mov ds,ax
	mov es,ax
	call inthandler21
	pop eax
	popad
	pop ds
	pop es
	iretd

asm_inthandler2c:
	push es
	push ds
	pushad
	mov eax,esp
	push eax
	mov ax,ss
	mov ds,ax
	mov es,ax
	call inthandler2c
	pop eax
	popad
	pop ds
	pop es
	iretd

memtest_sub:		; unsigned int memtest_sub(unsigned int start, unsigned int end);
	push edi
	push esi
	push ebx
	mov eax,[esp+12+4]			; unsigned int i = start;
	mov esi,0xaa55aa55			; unsigned int pat0 = 0xaa55aa55;
	mov edi,0x55aa55aa			; unsigned int pat1 = 0x55aa55aa;
.loop:
	mov ebx,eax
	add ebx,0xffc				; unsigned int *p = i + 0x0ffc;
	mov edx,[ebx]				; unsigned int old = *p;
	mov [ebx],esi				; *p = pat0;
	xor dword [ebx],0xffffffff	; *p ^= 0xffffffff;
	cmp edi,[ebx]				; if (*p != pat1)
	jne .fin					;   goto .fin;
	xor dword [ebx],0xffffffff	; *p ^= 0xffffffff;
	cmp esi,[ebx]				; if (*p != pat0)
	jne .fin					;   goto .fin;
	mov [ebx],edx				; *p = old;
	add eax,0x1000				; i += 0x1000;
	cmp eax,[esp+12+8]			; if (i < end)
	jb .loop					;   goto .loop;
	pop ebx
	pop esi
	pop edi
	ret
.fin:
	mov [ebx],edx				; *p = old;
	pop ebx
	pop esi
	pop edi
	ret

taskswitch3:		; void taskswitch3(void);
	jmp 3*8:0
	ret

taskswitch4:		; void taskswitch4(void);
	jmp 4*8:0
	ret
