#include "mystdio.h"
#include "bootpack.h"

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADDR_BOOTINFO;
	char s[40], mcursor[256];
	unsigned char keybuf[32], mousebuf[128];
	struct MOUSE_DEC mdec;
	int mx, my, i;

	init_gdtidt();

	init_pic();

	io_sti();
	io_out8(PORT_PIC0_DATA, 0xf9);	/* enable PIC1 and PS/2 keyboard (0b11111001) */
	io_out8(PORT_PIC1_DATA, 0xef);	/* enable PS/2 mouse (0b11101111) */

	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	init_keyboard();
	enable_mouse(&mdec);

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor(mcursor, COL8_008484);
	putblock(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor);
	mysprintf(s, "(%3d, %3d)", mx, my);
	putfonts(binfo->vram, binfo->scrnx, binfo->fonts, 0, 0, COL8_FFFFFF, s);

	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo)) {
				i = fifo8_get(&keyfifo);
				io_sti();
				mysprintf(s, "%02X", i);
				boxfill(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 16, 32);
				putfonts(binfo->vram, binfo->scrnx, binfo->fonts, 0, 16, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo)) {
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
					mysprintf(s, "[lcr] %4d %4d", mdec.x, mdec.y);
					if (mdec.btn & 0x01)
						s[1] = 'L';
					if (mdec.btn & 0x02)
						s[3] = 'R';
					if (mdec.btn & 0x04)
						s[2] = 'C';
					boxfill(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8, 32);
					putfonts(binfo->vram, binfo->scrnx, binfo->fonts, 32, 16, COL8_FFFFFF, s);

					boxfill(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 80, 16);
					boxfill(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 16, my + 16);
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0)
						mx = 0;
					if (my < 0)
						my = 0;
					if (mx > binfo->scrnx - 16)
						mx = binfo->scrnx - 16;
					if (my > binfo->scrny - 16)
						my = binfo->scrny - 16;
					mysprintf(s, "(%3d, %3d)", mx, my);
					putfonts(binfo->vram, binfo->scrnx, binfo->fonts, 0, 0, COL8_FFFFFF, s);
					putblock(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor);
				}
			}
		}
	}
}
