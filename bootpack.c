#include "mystdio.h"
#include "bootpack.h"

void make_window(char *buf, int xsize, int ysize, char *title);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADDR_BOOTINFO;
	char s[40];
	unsigned char keybuf[32], mousebuf[128];
	struct MOUSE_DEC mdec;
	int mx, my, i;
	unsigned int memtotal, count = 0;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	char *buf_back, buf_mouse[256], *buf_win;

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
	sht_win = sheet_alloc(shtctl);
	buf_back = (char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_win = (char *)memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, buf_win, 160, 52, -1);
	init_screen(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse_cursor(buf_mouse, 99);
	make_window(buf_win, 160, 52, "counter");
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_win, 1);
	sheet_updown(sht_mouse, 2);

	/* Print info */
	mysprintf(s, "(%3d, %3d)", mx, my);
	putfonts(buf_back, binfo->scrnx, binfo->fonts, 0, 0, COL8_FFFFFF, s);
	mysprintf(s, "MemTotal: %d MB, MemFree: %d KB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts(buf_back, binfo->scrnx, binfo->fonts, 0, 32, COL8_FFFFFF, s);
	sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);

	for (;;) {
		++count;
		mysprintf(s, "%010d", count);
		boxfill(buf_win, 160, COL8_C6C6C6, 40, 28, 120, 44);
		putfonts(buf_win, 160, binfo->fonts, 40, 28, COL8_000000, s);
		sheet_refresh(sht_win, 40, 28, 120, 44);

		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_sti();
		} else {
			if (fifo8_status(&keyfifo)) {
				i = fifo8_get(&keyfifo);
				io_sti();
				mysprintf(s, "%02X", i);
				boxfill(buf_back, binfo->scrnx, COL8_008484, 0, 16, 16, 32);
				putfonts(buf_back, binfo->scrnx, binfo->fonts, 0, 16, COL8_FFFFFF, s);
				sheet_refresh(sht_back, 0, 16, 16, 32);
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
					sheet_refresh(sht_back, 32, 16, 32 + 15 * 8, 32);

					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) mx = 0;
					if (my < 0) my = 0;
					if (mx >= binfo->scrnx) mx = binfo->scrnx - 1;
					if (my >= binfo->scrny) my = binfo->scrny - 1;
					mysprintf(s, "(%3d, %3d)", mx, my);
					boxfill(buf_back, binfo->scrnx, COL8_008484, 0, 0, 80, 16);
					putfonts(buf_back, binfo->scrnx, binfo->fonts, 0, 0, COL8_FFFFFF, s);
					sheet_refresh(sht_back, 0, 0, 80, 16);
					sheet_slide(sht_mouse, mx, my);
				}
			}
		}
	}
}

void make_window(char *buf, int xsize, int ysize, char *title)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADDR_BOOTINFO;
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;

	/* window frame */
	boxfill(buf, xsize, COL8_C6C6C6, 0,         0,         xsize,     1        );
	boxfill(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 1, 2        );
	boxfill(buf, xsize, COL8_C6C6C6, 0,         0,         1,         ysize    );
	boxfill(buf, xsize, COL8_FFFFFF, 1,         1,         2,         ysize - 1);
	boxfill(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 1, ysize - 1);
	boxfill(buf, xsize, COL8_000000, xsize - 1, 0,         xsize,     ysize    );
	boxfill(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 1, ysize - 1);
	boxfill(buf, xsize, COL8_000000, 0,         ysize - 1, xsize,     ysize    );
	/* body area */
	boxfill(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 2, ysize - 2);
	/* title area */
	boxfill(buf, xsize, COL8_000084, 3,         3,         xsize - 3, 21       );
	putfonts(buf, xsize, binfo->fonts, 24, 4, COL8_FFFFFF, title);
	for (y = 0; y < 14; ++y) {
		for (x = 0; x < 16; ++x) {
			c = closebtn[y][x];

			if (c == '@') c = COL8_000000;
			else if (c == '$') c = COL8_848484;
			else if (c == 'Q') c = COL8_C6C6C6;
			else c = COL8_FFFFFF;

			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
}