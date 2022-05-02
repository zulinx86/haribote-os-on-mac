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
	for (i = 0; i < MAX_TIMERS; ++i)
		timerctl.timer[i].flag = TIMER_FLAG_UNUSE;
}

struct TIMER *timer_alloc(void)
{
	int i;

	for (i = 0; i < MAX_TIMERS; ++i) {
		if (timerctl.timer[i].flag == TIMER_FLAG_UNUSE) {
			timerctl.timer[i].flag = TIMER_FLAG_ALLOC;
			return &timerctl.timer[i];
		}
	}

	return 0;
}

void timer_free(struct TIMER *timer)
{
	timer->flag = TIMER_FLAG_UNUSE;
}

void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data)
{
	timer->fifo = fifo;
	timer->data = data;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	timer->timeout = timeout + timerctl.count;
	timer->flag = TIMER_FLAG_USING;
	if (timer->timeout < timerctl.next)
		timerctl.next = timer->timeout;
}

void inthandler20(int *esp)
{
	int i;

	++timerctl.count;
	if (timerctl.next > timerctl.count)
		goto eoi;

	timerctl.next = 0xffffffff;
	for (i = 0; i < MAX_TIMERS; ++i) {
		if (timerctl.timer[i].flag == TIMER_FLAG_USING) {
			if (timerctl.timer[i].timeout <= timerctl.count) {
				timerctl.timer[i].flag = TIMER_FLAG_ALLOC;
				fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
			} else {
				if (timerctl.timer[i].timeout < timerctl.next)
					timerctl.next = timerctl.timer[i].timeout;
			}
		}
	}

eoi:
	io_out8(PORT_PIC0_COMM, PIC0_EOI_TIMER);
}
