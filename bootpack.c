#include "mystdio.h"
#include "bootpack.h"

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADDR_BOOTINFO;
	char s[40];
	unsigned char keybuf[32], mousebuf[128];
	struct MOUSE_DEC mdec;
	int mx, my, i;
	unsigned int memtotal;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse;
	char *buf_back, buf_mouse[256];

	/* Initialize hardwares */
	init_gdtidt();
	init_pic();
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_sti();
	io_out8(PORT_PIC0_DATA, 0xf9);	/* enable PIC1 and PS/2 keyboard (0b11111001) */
	io_out8(PORT_PIC1_DATA, 0xef);	/* enable PS/2 mouse (0b11101111) */
	init_keyboard();
	enable_mouse(&mdec);

	/* Initialize memory manager */
	memtotal = memtest(0x00400000, 0xc0000000);
	memman_init(memman);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	/* Initialize screen */
	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	buf_back = (char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_screen(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse_cursor(buf_mouse, 99);
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(shtctl, sht_back, 0, 0);
	sheet_slide(shtctl, sht_mouse, mx, my);
	sheet_updown(shtctl, sht_back, 0);
	sheet_updown(shtctl, sht_mouse, 1);

	/* Print info */
	mysprintf(s, "(%3d, %3d)", mx, my);
	putfonts(buf_back, binfo->scrnx, binfo->fonts, 0, 0, COL8_FFFFFF, s);
	mysprintf(s, "MemTotal: %d MB, MemFree: %d KB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts(buf_back, binfo->scrnx, binfo->fonts, 0, 32, COL8_FFFFFF, s);
	sheet_refresh(shtctl);

	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo)) {
				i = fifo8_get(&keyfifo);
				io_sti();
				mysprintf(s, "%02X", i);
				boxfill(buf_back, binfo->scrnx, COL8_008484, 0, 16, 16, 32);
				putfonts(buf_back, binfo->scrnx, binfo->fonts, 0, 16, COL8_FFFFFF, s);
				sheet_refresh(shtctl);
			} else if (fifo8_status(&mousefifo)) {
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
					mysprintf(s, "[lcr] %4d %4d", mdec.x, mdec.y);
					if (mdec.btn & 0x01) s[1] = 'L';
					if (mdec.btn & 0x02) s[3] = 'R';
					if (mdec.btn & 0x04) s[2] = 'C';
					boxfill(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8, 32);
					putfonts(buf_back, binfo->scrnx, binfo->fonts, 32, 16, COL8_FFFFFF, s);

					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) mx = 0;
					if (my < 0) my = 0;
					if (mx > binfo->scrnx - 16) mx = binfo->scrnx - 16;
					if (my > binfo->scrny - 16) my = binfo->scrny - 16;
					mysprintf(s, "(%3d, %3d)", mx, my);
					boxfill(buf_back, binfo->scrnx, COL8_008484, 0, 0, 80, 16);
					putfonts(buf_back, binfo->scrnx, binfo->fonts, 0, 0, COL8_FFFFFF, s);
					sheet_slide(shtctl, sht_mouse, mx, my);
				}
			}
		}
	}
}
