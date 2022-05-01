#include "bootpack.h"

struct TIMERCTL timerctl;

void init_pit(void)
{
	io_out8(PORT_PIT_COMM, PIT_CHANNEL0 | PIT_LOHI | PIT_RATEGEN | PIT_BINARY);
	/* 0x2e9c = 11932 => 100 Hz = every 10 ms */
	io_out8(PORT_PIT_CNT0, 0x9c);
	io_out8(PORT_PIT_CNT0, 0x2e);

	timerctl.count = 0;
}

void inthandler20(int *esp)
{
	++timerctl.count;
	io_out8(PORT_PIC0_COMM, PIC0_EOI_TIMER);
}