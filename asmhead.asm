; haribote-os

; Boot info

	cyls  equ 0x0ff0
	leds  equ 0x0ff1
	vmode equ 0x0ff2
	scrnx equ 0x0ff4
	scrny equ 0x0ff6
	vram  equ 0x0ff8
	fonts equ 0x0ffc

; I/O port

	pic0_comm equ 0x20
	pic0_data equ 0x21
	pic1_comm equ 0xa0
	pic1_data equ 0xa1

	kbc_data equ 0x60
	kbc_comm equ 0x64
	kbc_stat equ 0x64

; Disk caches

	bootpack_cache equ 0x00280000	; where bootpack is loaded
	realmode_cache equ 0x00008000	; where disk cache is placed in real mode
	protmode_cache equ 0x00100000	; where disk cache is placed in protected mode

; VBE (Video BIOS Extensions)

	vbemode equ 0x0103
	; 0x0100 :  640 x  400 x 8 bit color
	; 0x0101 :  640 x  480 x 8 bit color
	; 0x0103 :  800 x  600 x 8 bit color
	; 0x0105 : 1024 x  768 x 8 bit color
	; 0x0107 : 1280 x 1024 x 8 bit color

; Program

	org 0xc200

	; Check the existence of VBE
	mov ax,0x9000
	mov es,ax
	mov di,0
	mov ax,0x4f00		; function to get controller info
	int 0x10			; BIOS interruption for video display
	cmp ax,0x004f
	jne scrn320

	; Check if VBE is greater than or equal to version 2.0
	mov ax,[es:di+4]
	cmp ax,0x0200
	jb scrn320

	; Check if the display mode is available
	mov cx,vbemode
	mov ax,0x4f01		; function to get mode info
	int 0x10			; BIOS interruption for video display
	cmp ax,0x004f
	jne scrn320

	; Check info about the display mode
	cmp byte [es:di+0x19],8
	jne scrn320			; 8 bit color or not
	cmp byte [es:di+0x1b],4
	jne scrn320			; palette mode or not
	mov ax,[es:di+0x00]
	and ax,0x0080		; able to be added to 0x4000 or not
	jz scrn320

	; Switch to the display mode
	mov bx,vbemode+0x4000
	mov ax,0x4f02		; function to set video mode
	int 0x10			; BIOS interruption for video display
	mov byte[vmode],8
	mov ax,[es:di+0x12]
	mov [scrnx],ax
	mov ax,[es:di+0x14]
	mov [scrny],ax
	mov eax,[es:di+0x28]
	mov [vram],eax
	jmp get_bitmap_fonts

	; VGA graphics (320 x 200 x 8 bit color)
scrn320:
	mov al,0x13
	mov ah,0x00
	int 0x10
	mov byte [vmode],8
	mov word [scrnx],320
	mov word [scrny],200
	mov dword [vram],0x000a0000

	; Get bitmap fonts from BIOS
get_bitmap_fonts:
	mov ah,0x11			; character generator routine
	mov al,0x30			; get current character generator information
	mov bh,0x06			; ROM 8x16 character table pointer
	int 0x10			; BIOS interruption for video display
						; es:bp = pointer to table
	mov eax,0
	mov ax,es
	shl eax,4			; eax = (es << 4)
	mov ebx,0
	mov bx,bp			; ebx = bp
	add eax,ebx			; eax += ebx
	mov [fonts],eax

	; Get keyboard flags
	mov ah,0x02			; function to read keyboard flags
	int 0x16			; BIOS interruption for keyboard
	mov [leds],al

	; Ingore interruptions
	mov al,0xff
	out pic0_data,al	; mask PIC0
	nop					; consecutive out instructions may fail in some processors
	out pic1_data,al	; mask PIC1
	cli					; disable interrupts

	; Set A20 gate
	call enable_a20

	; Go to protected mode
	lgdt [gdtr0]
	mov eax,cr0
	and eax,0x7fffffff	; clear bit 41 (prohibit paging)
	or eax,0x00000001	; set bit 0 (go to protected mode)
	mov cr0,eax
	jmp pipeline_flush
pipeline_flush:

	; Initialize segment registers
	mov ax,1*8
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov ss,ax

	; Transfer bootpack
	mov esi,bootpack		; source address
	mov edi,bootpack_cache	; destination address
	mov ecx,512*1024/4		; size (4-byte unit)
	call memcpy

	; Transfer boot sector
	mov esi,0x7c00
	mov edi,protmode_cache
	mov ecx,512/4
	call memcpy

	; Transfer other disk cache
	mov esi,realmode_cache+512
	mov edi,protmode_cache+512
	mov ecx,0
	mov cl,[cyls]
	imul ecx,512*18*2/4
	sub ecx,512/4
	call memcpy

	; Go to bootpack
	mov ebx,bootpack
	mov ecx,[ebx+0x10]		; ecx = size of .data section (1-byte unit)
	add ecx,4				; ecx += 3
	shr ecx,2				; ecx /= 4 (4-byte unit)
	jz skip
	mov esi,[ebx+0x14]		; esi = file offset of .data section
	add esi,ebx				; esi += memory address of disk cache of bootpack
	mov edi,[ebx+0x0c]		; destination address of .data
	call memcpy				; copy .data section
skip:
	mov esp,[ebx+0x0c]
	jmp dword 2*8:0x0000001b


; Utilities

memcpy:
	mov eax,[esi]
	add esi,4
	mov [edi],eax
	add edi,4
	sub ecx,1
	jnz memcpy
	ret


; A20-related functions

	; Check if A20 line is enabled
	; Returns:
	; - 0: in ax if the a20 line is disabled (memory wraps around).
	; - 1: in ax if the a20 line is enabled (memory does not wrap around).
check_a20:
	pushf
	push ds
	push es
	push di
	push si

	xor ax,ax
	mov es,ax		; es = 0x0000
	not ax
	mov ds,ax		; ds = 0xffff

	mov di,0x0500	; di = 0x0500
	mov si,0x0510	; si = 0x0510

	mov al,[es:di]	; al = [es:di] (0x000500)
	push ax
	mov al,[ds:si]	; al = [ds:si] (0x100500)
	push ax

	mov byte [es:di],0x00
	mov byte [ds:si],0xff

	cmp byte [es:di],0xff

	pop ax
	mov [ds:si],al

	pop ax
	mov [es:di],al

	mov ax,0
	je .exit

	mov ax,1

.exit:
	pop si
	pop di
	pop es
	pop ds
	popf
	
	ret


	; Enable A20 line
enable_a20:
	call check_a20
	cmp al,0
	jz .exit

	call wait_kbc_out
	mov al,0xd0			; function to read controller output port
	out kbc_comm,al		; send command

	call wait_kbc_in
	in al,kbc_data		; read data
	push eax

	call wait_kbc_out
	mov al,0xd1			; function to write controller out port
	out kbc_comm,al		; send command

	call wait_kbc_out
	pop eax
	or al,2				; enable a20 bit
	out kbc_data,al

	call wait_kbc_out
.exit:
	ret


wait_kbc_in:
	in al,kbc_stat
	test al,1
	jz wait_kbc_in
	ret


wait_kbc_out:
	in al,kbc_stat
	test al,2
	jnz wait_kbc_out
	ret


; GDT (Global Descriptor Table)

	alignb 16
gdt0:
	dq 0x00_0_0_00_000000_0000
	dq 0x00_c_f_92_000000_ffff	; 32-bit r/w segment
	dq 0x00_c_f_9a_280000_ffff	; 32-bit executable segment (for bootpack)

gdtr0:
	dw 8*3-1
	dd gdt0

	alignb 16
bootpack:
