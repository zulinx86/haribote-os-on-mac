#include "bootpack.h"

void init_pic(void)
{
	io_out8(PORT_PIC0_DATA, 0xff);			/* ignore all interrupts */
	io_out8(PORT_PIC1_DATA, 0xff);			/* ignore all interrupts */

	io_out8(PORT_PIC0_COMM, ICW1_INIT | ICW1_ICW4);	/* edge trigger mode */
	io_out8(PORT_PIC0_DATA, 0x20);			/* receive IRQ0-7 at INT20-27 */
	io_out8(PORT_PIC0_DATA, 1 << 2);		/* connect PIC1 with IRQ2 */
	io_out8(PORT_PIC0_DATA, ICW4_8086);		/* 8086/88 mode */

	io_out8(PORT_PIC1_COMM, ICW1_INIT | ICW1_ICW4);	/* edge trigger mode */
	io_out8(PORT_PIC1_DATA, 0x28);			/* receive IRQ8-15 at INT28-2f */
	io_out8(PORT_PIC1_DATA, 2);			/* conenct PIC1 with IRQ2 */
	io_out8(PORT_PIC1_DATA, ICW4_8086);		/* 8086/88 mode */

	io_out8(PORT_PIC0_DATA, 0xfb);			/* ignore interrupts except for PIC1 */
	io_out8(PORT_PIC1_DATA, 0xff);			/* ignore all interrupts */
}