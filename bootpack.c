void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

#define PORT_VIDEO_WRITE 0x03c8
#define PORT_VIDEO_DATA  0x03c9

#define COL8_000000	0
#define COL8_FF0000	1
#define COL8_00FF00	2
#define COL8_FFFF00	3
#define COL8_0000FF	4
#define COL8_FF00FF	5
#define COL8_00FFFF	6
#define COL8_FFFFFF	7
#define COL8_C6C6C6	8
#define	COL8_840000	9
#define COL8_008400	10
#define COL8_848400	11
#define COL8_000084	12
#define COL8_840084	13
#define COL8_008484	14
#define COL8_848484	15

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill(char *vram, int xsize, char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int xsize, int ysize);

void HariMain(void)
{
	int xsize, ysize;
	char *vram;
	short *binfo_scrnx, *binfo_scrny;
	int *binfo_vram;

	init_palette();
	binfo_scrnx = (short *)0x0ff4;
	binfo_scrny = (short *)0x0ff6;
	binfo_vram = (int *)0x0ff8;
	xsize = *binfo_scrnx;
	ysize = *binfo_scrny;
	vram = (char *)*binfo_vram;

	init_screen(vram, xsize, ysize);

	for (;;)
		io_hlt();
}

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