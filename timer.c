#include "bootpack.h"

struct TIMERCTL timerctl;

void init_pit(void)
{
	int i;
	struct TIMER *t;

	io_out8(PORT_PIT_COMM, PIT_CHANNEL0 | PIT_LOHI | PIT_RATEGEN | PIT_BINARY);
	/* 0x2e9c = 11932 => 100 Hz = every 10 ms */
	io_out8(PORT_PIT_CNT0, 0x9c);
	io_out8(PORT_PIT_CNT0, 0x2e);

	timerctl.count = 0;
	for (i = 0; i < MAX_TIMERS; ++i)
		timerctl.timers0[i].flag = TIMER_FLAG_UNUSE;

	/* sentinel */
	t = timer_alloc();
	t->timeout = 0xffffffff;
	t->flag = TIMER_FLAG_USING;
	t->next = 0;
	timerctl.t0 = t;
	timerctl.next = 0xffffffff;
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
	int eflags;
	struct TIMER *t, *s;

	timer->timeout = timeout + timerctl.count;
	timer->flag = TIMER_FLAG_USING;

	eflags = io_load_eflags();
	io_cli();

	for (s = 0, t = timerctl.t0; t != 0; s = t, t = t->next) {
		if (timer->timeout <= t->timeout) {
			if (s == 0) {
				timerctl.t0 = timer;
				timerctl.next = timer->timeout;
			} else {
				s->next = timer;
			}
			timer->next = t;
			break;
		}
	}

	io_store_eflags(eflags);
}

void inthandler20(int *esp)
{
	struct TIMER *timer;

	++timerctl.count;
	if (timerctl.next > timerctl.count)
		goto end;

	for (timer = timerctl.t0; timer != 0; timer = timer->next) {
		if (timer->timeout > timerctl.count)
			break;
		timer->flag = TIMER_FLAG_ALLOC;
		fifo32_put(timer->fifo, timer->data);
	}

	timerctl.t0 = timer;
	timerctl.next = timerctl.t0->timeout;

end:
	io_out8(PORT_PIC0_COMM, PIC0_EOI_TIMER);
}
