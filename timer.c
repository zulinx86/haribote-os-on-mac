#include "bootpack.h"

struct TIMERCTL timerctl;

void init_pit(void)
{
	int i;

	io_out8(PORT_PIT_COMM, PIT_CHANNEL0 | PIT_LOHI | PIT_RATEGEN | PIT_BINARY);
	/* 0x2e9c = 11932 => 100 Hz = every 10 ms */
	io_out8(PORT_PIT_CNT0, 0x9c);
	io_out8(PORT_PIT_CNT0, 0x2e);

	timerctl.count = 0;
	timerctl.next = 0xffffffff;
	timerctl.using = 0;
	for (i = 0; i < MAX_TIMERS; ++i)
		timerctl.timers0[i].flag = TIMER_FLAG_UNUSE;
}

struct TIMER *timer_alloc(void)
{
	int i;

	for (i = 0; i < MAX_TIMERS; ++i) {
		if (timerctl.timers0[i].flag == TIMER_FLAG_UNUSE) {
			timerctl.timers0[i].flag = TIMER_FLAG_ALLOC;
			return &timerctl.timers0[i];
		}
	}

	return 0;
}

void timer_free(struct TIMER *timer)
{
	timer->flag = TIMER_FLAG_UNUSE;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int eflags, i, j;

	timer->timeout = timeout + timerctl.count;
	timer->flag = TIMER_FLAG_USING;

	eflags = io_load_eflags();
	io_cli();

	for (i = 0; i < timerctl.using; ++i)
		if (timerctl.timers[i]->timeout >= timer->timeout)
			break;

	for (j = timerctl.using; j > i; --j)
		timerctl.timers[j] = timerctl.timers[j - 1];

	++timerctl.using;
	timerctl.timers[i] = timer;
	timerctl.next = timerctl.timers[0]->timeout;

	io_store_eflags(eflags);
}

void inthandler20(int *esp)
{
	int i, j;

	++timerctl.count;
	if (timerctl.next > timerctl.count)
		goto eoi;

	for (i = 0; i < timerctl.using; ++i) {
		if (timerctl.timers[i]->timeout > timerctl.count)
			break;

		timerctl.timers[i]->flag = TIMER_FLAG_ALLOC;
		fifo32_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
	}

	timerctl.using -= i;
	for (j = 0; j < timerctl.using; ++j)
		timerctl.timers[j] = timerctl.timers[i + j];

	if (timerctl.using > 0)
		timerctl.next = timerctl.timers[0]->timeout;
	else
		timerctl.next = 0xffffffff;

eoi:
	io_out8(PORT_PIC0_COMM, PIC0_EOI_TIMER);
}
