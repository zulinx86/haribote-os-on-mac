#include "mystdio.h"
#include "bootpack.h"

extern struct KEYBUF keybuf;

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADDR_BOOTINFO;
	char s[40], mcursor[256];
	int mx, my, i;

	init_gdtidt();

	init_pic();
	io_sti();
	io_out8(PORT_PIC0_DATA, 0xf9);	/* enable PIC1 and PS/2 keyboard (0b11111001) */
	io_out8(PORT_PIC1_DATA, 0xef);	/* enable PS/2 mouse (0b11101111) */

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor(mcursor, COL8_008484);
	putblock(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor);
	mysprintf(s, "(%d, %d)", mx, my);
	putfonts(binfo->vram, binfo->scrnx, binfo->fonts, 0, 0, COL8_FFFFFF, s);

	for (;;) {
		io_cli();
		if (keybuf.flag == 0) {
			io_stihlt();
		} else {
			i = keybuf.data;
			keybuf.flag = 0;
			io_sti();
			mysprintf(s, "%02X", i);
			boxfill(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 16, 32);
			putfonts(binfo->vram, binfo->scrnx, binfo->fonts, 0, 16, COL8_FFFFFF, s);
		}
	}
}
