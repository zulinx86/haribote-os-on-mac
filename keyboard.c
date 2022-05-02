#include "bootpack.h"

struct FIFO32 *keyfifo;
int keybase;

void inthandler21(int *esp)
{
	int data;

	data = io_in8(PORT_KBC_DATA);
	io_out8(PORT_PIC0_COMM, PIC0_EOI_KEY);
	fifo32_put(keyfifo, data + keybase);
}

void wait_kbc_sendready(void)
{
	for (;;)
		if ((io_in8(PORT_KBC_STAT) & KBC_STAT_SEND_NOTREADY) == 0)
			break;
}

void init_keyboard(struct FIFO32 *fifo, int base)
{
	keyfifo = fifo;
	keybase = base;
	wait_kbc_sendready();
	io_out8(PORT_KBC_COMM, KBC_COMM_WRITE_CONFIG);
	wait_kbc_sendready();
	io_out8(PORT_KBC_DATA, KBC_CONFIG_BYTE);
}
