OUTPUT_FORMAT("binary");

SECTIONS
{
	.head 0x0 : {
		LONG(0)					/* 0x00: data segment size for app */
		LONG(0x69726148)		/* 0x04: signature "Hari" */
		LONG(0)					/* 0x08: auxiliary area of data segment */
		LONG(0x310000)			/* 0x0c: initial value of ESP and destination address of .data */
		LONG(SIZEOF(.data))		/* 0x10: size of .data */
		LONG(LOADADDR(.data))	/* 0x14: file offset of .data */
		LONG(0xe9000000)		/* 0x18: jmp instruction */
		LONG(HariMain - 0x20)	/* 0x1c: entry address */
		LONG(0)					/* 0x20: address of heap region */
	}

	.text : { *(.text) }

	.data 0x310000 : AT ( ADDR(.text) + SIZEOF(.text) ) {
		*(.data)
		*(.rodata)
		*(.bss)
	}

	/DISCARD/ : { *(.eh_frame) }
}
