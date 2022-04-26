; nasmfunc

bits 32

	global io_hlt

section .text

io_hlt:	; void io_hlt(void);
	hlt
	ret
