#include "mystdio.h"
#include "bootpack.h"

struct FIFO8 keyfifo, mousefifo;

void init_pic(void)
{
	io_out8(PORT_PIC0_DATA, 0xff);			/* ignore all interrupts */
	io_out8(PORT_PIC1_DATA, 0xff);			/* ignore all interrupts */

	io_out8(PORT_PIC0_COMM, ICW1_INIT | ICW1_ICW4);	/* ICW1: edge trigger mode & cascading mode & ICW4 is needed */
	io_out8(PORT_PIC0_DATA, 0x20);			/* ICW2: receive IRQ0-7 at INT20-27 */
	io_out8(PORT_PIC0_DATA, 1 << 2);		/* ICW3: connect PIC1 with IRQ2 */
	io_out8(PORT_PIC0_DATA, ICW4_8086);		/* ICW4: 8086/88 mode */

	io_out8(PORT_PIC1_COMM, ICW1_INIT | ICW1_ICW4);	/* ICW1: edge trigger mode & cascading mode & ICW4 is needed */
	io_out8(PORT_PIC1_DATA, 0x28);			/* ICW2: receive IRQ8-15 at INT28-2f */
	io_out8(PORT_PIC1_DATA, 2);			/* ICW3: conenct PIC1 with IRQ2 */
	io_out8(PORT_PIC1_DATA, ICW4_8086);		/* ICW4: 8086/88 mode */

	io_out8(PORT_PIC0_DATA, 0xfb);			/* ignore interrupts except for PIC1 */
	io_out8(PORT_PIC1_DATA, 0xff);			/* ignore all interrupts */
}

void inthandler21(int *esp)
{
	unsigned char data;

	data = io_in8(PORT_KBC_DATA);
	io_out8(PORT_PIC0_COMM, PIC0_EOI_KEY);
	fifo8_put(&keyfifo, data);
}

void inthandler2c(int *esp)
{
	unsigned char data;

	data = io_in8(PORT_KBC_DATA);
	io_out8(PORT_PIC1_COMM, PIC1_EOI_MOUSE);
	io_out8(PORT_PIC0_COMM, PIC0_EOI_PIC1);
	fifo8_put(&mousefifo, data);
}