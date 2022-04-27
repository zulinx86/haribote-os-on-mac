#include "mystdio.h"
#include "bootpack.h"

extern struct FIFO8 keyfifo, mousefifo;
void enable_mouse(void);
void init_keyboard(void);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADDR_BOOTINFO;
	char s[40], mcursor[256];
	unsigned char keybuf[32], mousebuf[128], mouse_dbuf[3], mouse_phase;
	int mx, my, i;

	init_gdtidt();

	init_pic();

	io_sti();
	io_out8(PORT_PIC0_DATA, 0xf9);	/* enable PIC1 and PS/2 keyboard (0b11111001) */
	io_out8(PORT_PIC1_DATA, 0xef);	/* enable PS/2 mouse (0b11101111) */

	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	mouse_phase = 0;
	init_keyboard();
	enable_mouse();

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
				if (mouse_phase == 0) {
					if (i == 0xfa)
						mouse_phase = 1;
				} else if (mouse_phase == 1) {
					mouse_dbuf[0] = i;
					mouse_phase = 2;
				} else if (mouse_phase == 2) {
					mouse_dbuf[1] = i;
					mouse_phase = 3;
				} else if (mouse_phase == 3) {
					mouse_dbuf[2] = i;
					mouse_phase = 1;

					mysprintf(s, "%02X %02X %02X", mouse_dbuf[0], mouse_dbuf[1], mouse_dbuf[2]);
					boxfill(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 8 * 8, 32);
					putfonts(binfo->vram, binfo->scrnx, binfo->fonts, 32, 16, COL8_FFFFFF, s);
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

void enable_mouse(void)
{
	wait_kbc_sendready();
	io_out8(PORT_KBC_COMM, KBC_COMM_TO_MOUSE);
	wait_kbc_sendready();
	io_out8(PORT_KBC_DATA, MOUSE_COMM_ENABLE);
	/* when it succeeds, ACK(0xfa) is sent */
}