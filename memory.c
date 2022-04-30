#include "bootpack.h"

#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flag486 = 0;
	unsigned int eflags, cr0, i;

	/* check whether the CPU is 386, or 486 and later. */
	eflags = io_load_eflags();
	eflags |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	io_store_eflags(eflags);
	eflags = io_load_eflags();
	if ((eflags & EFLAGS_AC_BIT) != 0) /* if the CPU is 386, AC is back to 0 automatically */
		flag486 = 1;
	eflags &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	io_store_eflags(eflags);

	if (flag486) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* disable CPU cache */
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flag486) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)
{
	man->frees = 0;		/* number of free spaces */
	man->maxfrees = 0;	/* max number of free spaces for tracking fragmentation */
	man->lostsize = 0;	/* sum of size of lost spaces */
	man->losts = 0;		/* number of lost spaces */
}

unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;

	for (i = 0; i < man->frees; ++i)
		t += man->free[i].size;

	return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;

	for (i = 0; i < man->frees; ++i) {
		if (man->free[i].size >= size) {
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				--man->frees;
				for (; i < man->frees; ++i) {
					man->free[i].addr = man->free[i + 1].addr;
					man->free[i].size = man->free[i + 1].size;
				}
			}
			return a;
		}
	}

	return 0;
}

unsigned int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;

	/* free[i - 1].addr < addr < free[i].addr */
	for (i = 0; i < man->frees; ++i) {
		if (man->free[i].addr > addr) break;
	}

	/* if it's not the head */
	if (i > 0) {
		/* merge with free[i - 1] */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			man->free[i - 1].size += size;
			/* merge with free[i] */
			if (addr + size == man->free[i].addr) {
				man->free[i - 1].size += man->free[i].size;
				--man->frees;
				for (; i < man->frees; ++i) {
					man->free[i].addr = man->free[i + 1].addr;
					man->free[i].size = man->free[i + 1].size;
				}
			}
			return 0;
		}
	}

	/* cannot merge it with free[i - 1] */
	if (i < man->frees) {
		/* merge with free[i] */
		if (addr + size == man->free[i].addr) {
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}

	/* cannot merge it with both free[i - 1] and free[i] */
	if (man->frees < MEMMAN_FREES) {
		for (j = man->frees; j > i; --j) {
			man->free[j].addr = man->free[j - 1].addr;
			man->free[j].size = man->free[j - 1].size;
		}

		++man->frees;
		if (man->maxfrees < man->frees)
			man->maxfrees = man->frees;

		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}

	++man->losts;
	man->lostsize += size;
	return 1;
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)
{
	unsigned int a;

	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);

	return a;
}

unsigned int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;

	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);

	return i;
}