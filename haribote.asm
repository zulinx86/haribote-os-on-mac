; haribote-os

; Boot info

	cyls  equ 0x0ff0
	leds  equ 0x0ff1
	vmode equ 0x0ff2
	scrnx equ 0x0ff4
	scrny equ 0x0ff6
	vram  equ 0x0ff8

; Program

	org 0xc200

	; Set video mode
	mov al,0x13		; 320x200 256-color graphics (VGA)
	mov ah,0x00		; function to set video mode
	int 0x10		; BIOS interruption for video display

	; Save boot info
	mov byte [vmode],8
	mov word [scrnx],320
	mov word [scrny],200
	mov dword [vram],0x000a0000

	; Get keyboard flags
	mov ah,0x02		; function to read keyboard flags
	int 0x16		; BIOS interruption for keyboard
	mov [leds],al

fin:
	hlt
	jmp fin
