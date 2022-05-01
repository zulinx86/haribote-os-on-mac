#include "bootpack.h"

#define SHEET_UNUSE	0
#define SHEET_USE	1

struct SHTCTL *shtctl_init(struct MEMMAN *memman, char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;

	ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if (ctl == 0) goto err;

	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = 0; /* no sheets */
	for (i = 0; i < MAX_SHEETS; ++i)
		ctl->sheets0[i].flags = SHEET_UNUSE;

err:
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;

	for (i = 0; i < MAX_SHEETS; ++i) {
		if (ctl->sheets0[i].flags == SHEET_UNUSE) {
			sht = &(ctl->sheets0[i]);
			sht->flags = SHEET_USE;
			sht->height = -1; /* not displayed */
			return sht;
		}
	}

	return 0; /* no available sheets */
}

void sheet_setbuf(struct SHEET *sht, char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
}

void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
	int h, old = sht->height;

	if (height > ctl->top) height = ctl->top;
	if (height < -1) height  = -1;
	sht->height = height;

	if (height < old) { /* the new height is lower than the old one */
		if (height >= 0) {
			for (h = old; h > height; --h) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else { /* set the sheet undisplayed */
			--ctl->top;
			for (h = old; h < ctl->top; ++h) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
		}
		sheet_refresh(ctl);
	} else if (height > old) { /* the new height is higher than the old one */
		if (old >= 0) {
			if (height == ctl->top) --height;
			for (h = old; h < height; ++h) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else { /* set the sheet displayed */
			for (h = ctl->top; h > height; --h) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
			++ctl->top;
		}
		sheet_refresh(ctl);
	}
}

void sheet_refresh(struct SHTCTL *ctl)
{
	int h, bx, by, vx, vy;
	char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;

	for (h = 0; h < ctl->top; ++h) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		for (by = 0; by < sht->bysize; ++by) {
			vy = sht->vy0 + by;
			for (bx = 0; bx < sht->bxsize; ++bx) {
				vx = sht->vx0 + bx;

				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_inv)
					vram[vy * ctl->xsize + vx] = c;
			}
		}
	}
}

void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0)
		sheet_refresh(ctl);
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	if (sht->height >= 0)
		sheet_updown(ctl, sht, -1);
	sht->flags = SHEET_UNUSE;
}