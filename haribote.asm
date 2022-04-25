; haribote-os

	org 0xc200

	; Set video mode
	mov al,0x13		; 320x200 256-color graphics (VGA)
	mov ah,0x00		; function to set video mode
	int 0x10		; BIOS interruption for video display

fin:
	hlt
	jmp fin
