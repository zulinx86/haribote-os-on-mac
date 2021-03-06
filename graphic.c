#include "bootpack.h"

void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0: bloack */
		0xff, 0x00, 0x00,	/*  1: bright red */
		0x00, 0xff, 0x00,	/*  2: bright green */
		0xff, 0xff, 0x00,	/*  3: bright yellow */
		0x00, 0x00, 0xff,	/*  4: bright blue */
		0xff, 0x00, 0xff,	/*  5: bright purple */
		0x00, 0xff, 0xff,	/*  6: bright light blue */
		0xff, 0xff, 0xff,	/*  7: white */
		0xc6, 0xc6, 0xc6,	/*  8: bright gray */
		0x84, 0x00, 0x00,	/*  9: dark red */
		0x00, 0x84, 0x00,	/* 10: dark green */
		0x84, 0x84, 0x00,	/* 11: dark yellow */
		0x00, 0x00, 0x84,	/* 12: dark blue */
		0x84, 0x00, 0x84,	/* 13: dark purple */
		0x00, 0x84, 0x84,	/* 14: dark light blue */
		0x84, 0x84, 0x84	/* 15: dark gray */
	};

	set_palette(0, 16, table_rgb);
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;

	eflags = io_load_eflags();
	io_cli();

	io_out8(PORT_VIDEO_WRITE, start);
	for (i = start; i < end * 3; ++i)
		io_out8(PORT_VIDEO_DATA, rgb[i] / 4);

	io_store_eflags(eflags);
}

void boxfill(char *vram, int xsize, char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y < y1; ++y)
		for (x = x0; x < x1; ++x)
			vram[y * xsize + x] = c;
	return;
}

void init_screen(char *vram, int xsize, int ysize)
{
	boxfill(vram, xsize, COL8_008484,          0,          0,      xsize, ysize - 28);
	boxfill(vram, xsize, COL8_C6C6C6,          0, ysize - 28,      xsize, ysize - 27);
	boxfill(vram, xsize, COL8_FFFFFF,          0, ysize - 27,      xsize, ysize - 26);
	boxfill(vram, xsize, COL8_C6C6C6,          0, ysize - 26,      xsize,      ysize);

	boxfill(vram, xsize, COL8_FFFFFF,          2, ysize - 24,         60, ysize - 23);
	boxfill(vram, xsize, COL8_FFFFFF,          2, ysize - 24,          3, ysize -  3);
	boxfill(vram, xsize, COL8_000000,          2, ysize -  3,         61, ysize -  2);
	boxfill(vram, xsize, COL8_000000,         60, ysize - 24,         61, ysize -  2);
	boxfill(vram, xsize, COL8_848484,          3, ysize -  4,         60, ysize -  3);
	boxfill(vram, xsize, COL8_848484,         59, ysize - 23,         60, ysize -  3);

	boxfill(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  3, ysize - 23);
	boxfill(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 46, ysize -  4);
	boxfill(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  4, xsize -  3, ysize -  3);
	boxfill(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  2, ysize -  3);
}

void putfont(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i, j;
	char *p, d;

	for (i = 0; i < 16; ++i) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		for (j = 0; j < 8; ++j)
			if (d & (1 << (7 - j)))
				p[j] = c;
	}
}

void putfonts(char *vram, int xsize, char *fonts, int x, int y, char c, const char *s)
{
	for (; *s != 0x00; ++s) {
		putfont(vram, xsize, x, y, c, fonts + (*s) * 16);
		x += 8;
	}
}

void init_mouse_cursor(char *mouse, char bc)
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;

	for (y = 0; y < 16; ++y) {
		for (x = 0; x < 16; ++x) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
}

void putblock(char *vram, int xsize, int pxsize, int pysize, int px0, int py0, char *buf)
{
	int x, y;

	for (y = 0; y < pysize; ++y) {
		for (x = 0; x < pxsize; ++x) {
			vram[(py0 + y) * xsize + (px0 + x)] = buf[y * pxsize + x];
		}
	}
}