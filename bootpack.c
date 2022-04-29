#include "mystdio.h"
#include "bootpack.h"

extern struct FIFO8 keyfifo, mousefifo;
struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};
void enable_mouse(struct MOUSE_DEC *mdec);
void init_keyboard(void);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data);

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


#define PORT_KBC_DATA		0x60
#define PORT_KBC_STAT		0x64
#define PORT_KBC_COMM		0x64
#define KBC_COMM_WRITE_CONFIG	0x60
#define KBC_COMM_TO_MOUSE	0xd4
#define KBC_CONFIG_BYTE		0x47
#define KBC_STAT_SEND_NOTREADY	0x02
#define MOUSE_COMM_ENABLE	0xf4

void wait_kbc_sendready(void)
{
	for (;;)
		if ((io_in8(PORT_KBC_STAT) & KBC_STAT_SEND_NOTREADY) == 0)
			break;
}

void init_keyboard(void)
{
	wait_kbc_sendready();
	io_out8(PORT_KBC_COMM, KBC_COMM_WRITE_CONFIG);
	wait_kbc_sendready();
	io_out8(PORT_KBC_DATA, KBC_CONFIG_BYTE);
}

void enable_mouse(struct MOUSE_DEC *mdec)
{
	mdec->phase = 0;

	wait_kbc_sendready();
	io_out8(PORT_KBC_COMM, KBC_COMM_TO_MOUSE);
	wait_kbc_sendready();
	io_out8(PORT_KBC_DATA, MOUSE_COMM_ENABLE);
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data)
{
	if (mdec->phase == 0) {
		if (data == 0xfa) /* ACK */
			mdec->phase = 1;
		return 0;
	} else if (mdec->phase == 1) {
		if ((data & 0xc8) == 0x08) { /* check if it is valid */
			mdec->buf[0] = data;
			mdec->phase = 2;
		}
		return 0;
	} else if (mdec->phase == 2) {
		mdec->buf[1] = data;
		mdec->phase = 3;
		return 0;
	} else if (mdec->phase == 3) {
		mdec->buf[2] = data;
		mdec->phase = 1;

		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if ((mdec->buf[0] & 0x10))
			mdec->x |= 0xffffff00;
		if ((mdec->buf[0] & 0x20))
			mdec->y |= 0xffffff00;
		mdec->y *= -1;
		return 1;
	}

	return -1;
}