#include "mystdio.h"
#include "bootpack.h"

void make_window(char *buf, int xsize, int ysize, char *title);
void putfonts_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADDR_BOOTINFO;
	char s[40];
	struct FIFO32 fifo;
	int fifobuf[128];
	struct MOUSE_DEC mdec;
	int mx, my, i;
	unsigned int memtotal;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	char *buf_back, buf_mouse[256], *buf_win;
	struct TIMER *timer1, *timer2, *timer3;

	/* Initialize hardwares */
	init_gdtidt();
	init_pic();
	io_sti();
	fifo32_init(&fifo, 128, fifobuf);
	init_pit();
	init_keyboard(&fifo, KEY_BASE);
	enable_mouse(&fifo, MOUSE_BASE, &mdec);
	io_out8(PORT_PIC0_DATA, 0xf8);	/* enable PIT, PIC1 and PS/2 keyboard (0b11111000) */
	io_out8(PORT_PIC1_DATA, 0xef);	/* enable PS/2 mouse (0b11101111) */

	/* Initialize timer */
	timer1 = timer_alloc();
	timer2 = timer_alloc();
	timer3 = timer_alloc();
	timer_init(timer1, &fifo, 10);
	timer_init(timer2, &fifo, 3);
	timer_init(timer3, &fifo, 1);
	timer_settime(timer1, 1000);
	timer_settime(timer2, 300);
	timer_settime(timer3, 50);

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
	make_window(buf_win, 160, 52, "window");
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
	putfonts_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	mysprintf(s, "MemTotal: %d MB, MemFree: %d KB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);

	for (;;) {
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_stihlt();
		} else {
			i = fifo32_get(&fifo);
			io_sti();

			if (KEY_BASE <= i && i < KEY_BASE + 256) {
				mysprintf(s, "%02X", i - KEY_BASE);
				putfonts_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				
				if (i == 0x1e + 256)
					putfonts_sht(sht_win, 40, 28, COL8_000000, COL8_C6C6C6, "A", 1);
			} else if (MOUSE_BASE <= i && i < MOUSE_BASE + 256) {
				if (mouse_decode(&mdec, i - MOUSE_BASE) != 0) {
					mysprintf(s, "[lcr] %4d %4d", mdec.x, mdec.y);
					if (mdec.btn & 0x01) s[1] = 'L';
					if (mdec.btn & 0x02) s[3] = 'R';
					if (mdec.btn & 0x04) s[2] = 'C';
					putfonts_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);

					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) mx = 0;
					if (my < 0) my = 0;
					if (mx >= binfo->scrnx) mx = binfo->scrnx - 1;
					if (my >= binfo->scrny) my = binfo->scrny - 1;
					mysprintf(s, "(%3d, %3d)", mx, my);
					putfonts_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					sheet_slide(sht_mouse, mx, my);
				}
			} else if (i == 10) {
				putfonts_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10 sec", 6);
			} else if (i == 3) {
				putfonts_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3 sec", 5);
			} else if (i == 1) {
				timer_init(timer3, &fifo, 0);
				boxfill(buf_back, binfo->scrnx, COL8_008484, 8, 96, 16, 112);
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 96, 16, 112);
			} else if (i == 0) {
				timer_init(timer3, &fifo, 1);
				boxfill(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 96, 16, 112);
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 96, 16, 112);
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

void putfonts_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADDR_BOOTINFO;
	boxfill(sht->buf, sht->bxsize, b, x, y, x + l * 8, y + 16);
	putfonts(sht->buf, sht->bxsize, binfo->fonts, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
}