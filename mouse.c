#include "bootpack.h"

struct FIFO32 *mousefifo;
int mousebase;

void inthandler2c(int *esp)
{
	int data;

	data = io_in8(PORT_KBC_DATA);
	io_out8(PORT_PIC1_COMM, PIC1_EOI_MOUSE);
	io_out8(PORT_PIC0_COMM, PIC0_EOI_PIC1);
	fifo32_put(mousefifo, data + mousebase);
}

void enable_mouse(struct FIFO32 *fifo, int base, struct MOUSE_DEC *mdec)
{
	mousefifo = fifo;
	mousebase = base;

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