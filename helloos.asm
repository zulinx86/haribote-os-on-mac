; helloos

	org 0x7c00

; Description for FAT12 floppy disk

	jmp short entry
	db 0x90				; nop

	; BIOS Parameter Block (BPB)
	; FAT12/16/32 common fileds
	db "HELLOIPL"		; OEM identifier
	dw 512				; Number of bytes per sector
	db 1				; Number of sectors per cluster
	dw 1				; Number of reserved sectors
	db 2				; Number of file allocation tables
	dw 224				; Root directory entries
	dw 2880				; Number of sectors (2880 means 1.4MB floppy)
	db 0xf0				; Media type (0xf0 means removable)
	dw 9				; Number of sectors per file allocation table
	dw 18				; Number of sectors per track
	dw 2				; Number of heads
	dd 0				; Number of hidden sectors
	dd 2880				; Number of sectors
	; Fields for FAT12/16
	db 0				; Drive number used by disk BIOS
	db 0				; Reserved
	db 0x29				; Extended boot signature
	dd 0xffffffff		; Volume serial number
	db "HELLO-OS   "	; 11-byte volume label
	db "FAT12   "		; FAT filesystem type
	times 18 db 0

; Program

entry:
	; Initialize segment registers
	mov ax,0
	mov ss,ax
	mov sp,0x7c00
	mov ds,ax
	mov es,ax

	; Display a message
	mov si,msg
putloop:
	mov al,[si]
	add si,1
	cmp al,0
	je fin
	mov ah,0xe
	mov bx,0x000f
	int 0x10
	jmp putloop
fin:
	hlt
	jmp fin

msg:
	db 0x0a,0x0a
	db "hello, world"
	db 0x0a
	db 0x00

	times 0x1fe-($-$$) db 0

	db 0x55,0xaa

; After boot sector

	db 0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x00
	times 4600 db 0
	db 0xf0,0xff,0xff,0x00,0x00,0x00,0x00,0x00
	times 1469432 db 0
