#include "bootpack.h"

struct FIFO8 keyfifo;

void inthandler21(int *esp)
{
	unsigned char data;

	data = io_in8(PORT_KBC_DATA);
	io_out8(PORT_PIC0_COMM, PIC0_EOI_KEY);
	fifo8_put(&keyfifo, data);
}

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
