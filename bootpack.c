#include "mystdio.h"
#include "bootpack.h"

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADDR_BOOTINFO;
	char s[40], mcursor[256];
	int mx, my;

	init_gdtidt();

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor(mcursor, COL8_008484);
	putblock(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor);
	mysprintf(s, "(%d, %d)", mx, my);
	putfonts(binfo->vram, binfo->scrnx, binfo->fonts, 0, 0, COL8_FFFFFF, s);

	for (;;)
		io_hlt();
}
