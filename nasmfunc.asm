; nasmfunc

bits 32

	global io_hlt
	global write_mem8

section .text

io_hlt:		; void io_hlt(void);
	hlt
	ret

write_mem8:	; void write_mem8(int addr, int data);
	mov ecx,[esp+4]
	mov al,[esp+8]
	mov [ecx],al
	ret
