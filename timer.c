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
	int eflags;
	struct TIMER *t, *s;

	timer->timeout = timeout + timerctl.count;
	timer->flag = TIMER_FLAG_USING;

	eflags = io_load_eflags();
	io_cli();

	++timerctl.using;
	if (timerctl.using == 1) {
		timerctl.t0 = timer;
		timer->next = 0;
		timerctl.next = timer->timeout;
		goto end;
	}

	t = timerctl.t0;
	if (timer->timeout <= t->timeout) {
		timerctl.t0 = timer;
		timer->next = t;
		timerctl.next = timer->timeout;
		goto end;
	}

	for (;;) {
		s = t;
		t = t->next;

		if (t == 0) break;
		if (timer->timeout <= t->timeout) {
			s->next = timer;
			timer->next = t;
			goto end;
		}
	}

	s->next = timer;
	timer->next = 0;

end:
	io_store_eflags(eflags);
}

void inthandler20(int *esp)
{
	int i;
	struct TIMER *timer;

	++timerctl.count;
	if (timerctl.next > timerctl.count)
		goto end;

	timer = timerctl.t0;
	for (i = 0; i < timerctl.using; ++i) {
		if (timer->timeout > timerctl.count)
			break;

		timer->flag = TIMER_FLAG_ALLOC;
		fifo32_put(timer->fifo, timer->data);
		timer = timer->next;
	}
	timerctl.t0 = timer;

	timerctl.using -= i;
	if (timerctl.using > 0)
		timerctl.next = timerctl.t0->timeout;
	else
		timerctl.next = 0xffffffff;

end:
	io_out8(PORT_PIC0_COMM, PIC0_EOI_TIMER);
}
