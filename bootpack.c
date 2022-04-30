#include "mystdio.h"
#include "bootpack.h"

#define MEMMAN_FREES	4094 /* 4096 - 2 */
#define MEMMAN_ADDR	0x003c0000

struct FREEINFO {
	unsigned int addr, size;
};

struct MEMMAN {
	unsigned int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
unsigned int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADDR_BOOTINFO;
	char s[40], mcursor[256];
	unsigned char keybuf[32], mousebuf[128];
	struct MOUSE_DEC mdec;
	int mx, my, i;
	unsigned int memtotal;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

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

	memtotal = memtest(0x00400000, 0xc0000000);
	memman_init(memman);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	mysprintf(s, "MemTotal: %d MB, MemFree: %d KB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts(binfo->vram, binfo->scrnx, binfo->fonts, 0, 32, COL8_FFFFFF, s);

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

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flag486 = 0;
	unsigned int eflags, cr0, i;

	/* check whether the CPU is 386, or 486 and later. */
	eflags = io_load_eflags();
	eflags |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	io_store_eflags(eflags);
	eflags = io_load_eflags();
	if ((eflags & EFLAGS_AC_BIT) != 0) /* if the CPU is 386, AC is back to 0 automatically */
		flag486 = 1;
	eflags &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	io_store_eflags(eflags);

	if (flag486) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* disable CPU cache */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flag486) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;		/* number of free spaces */
	man->maxfrees = 0;	/* max number of free spaces for tracking fragmentation */
	man->lostsize = 0;	/* sum of size of lost spaces */
	man->losts = 0;		/* number of lost spaces */
}

unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;

	for (i = 0; i < man->frees; ++i)
		t += man->free[i].size;

	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;

	for (i = 0; i < man->frees; ++i) {
		if (man->free[i].size >= size) {
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				--man->frees;
				for (; i < man->frees; ++i) {
					man->free[i].addr = man->free[i + 1].addr;
					man->free[i].size = man->free[i + 1].size;
				}
			}
			return a;
		}
	}

	return 0;
}

unsigned int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;

	/* free[i - 1].addr < addr < free[i].addr */
	for (i = 0; i < man->frees; ++i) {
		if (man->free[i].addr > addr) break;
	}

	/* if it's not the head */
	if (i > 0) {
		/* merge with free[i - 1] */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			man->free[i - 1].size += size;
			/* merge with free[i] */
			if (addr + size == man->free[i].addr) {
				man->free[i - 1].size += man->free[i].size;
				--man->frees;
				for (; i < man->frees; ++i) {
					man->free[i].addr = man->free[i + 1].addr;
					man->free[i].size = man->free[i + 1].size;
				}
			}
			return 0;
		}
	}

	/* cannot merge it with free[i - 1] */
	if (i < man->frees) {
		/* merge with free[i] */
		if (addr + size == man->free[i].addr) {
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}

	/* cannot merge it with both free[i - 1] and free[i] */
	if (man->frees < MEMMAN_FREES) {
		for (j = man->frees; j > i; --j) {
			man->free[j].addr = man->free[j - 1].addr;
			man->free[j].size = man->free[j - 1].size;
		}

		++man->frees;
		if (man->maxfrees < man->frees)
			man->maxfrees = man->frees;

		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}

	++man->losts;
	man->lostsize += size;
	return 1;
}