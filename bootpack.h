/* asmhead.asm */
#define ADDR_BOOTINFO	0x0ff0

struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram, *fonts;
};

/* nasmfunc.asm */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
int load_cr0(void);
void store_cr0(int cr0);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler2c(void);
unsigned int memtest_sub(unsigned int start, unsigned int end);

/* fifo.c */
struct FIFO8 {
	unsigned char *buf;
	int p, q, size, free, flags;
};

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);

/* graphic.c */
#define PORT_VIDEO_WRITE 0x03c8
#define PORT_VIDEO_DATA  0x03c9

#define COL8_000000	0
#define COL8_FF0000	1
#define COL8_00FF00	2
#define COL8_FFFF00	3
#define COL8_0000FF	4
#define COL8_FF00FF	5
#define COL8_00FFFF	6
#define COL8_FFFFFF	7
#define COL8_C6C6C6	8
#define	COL8_840000	9
#define COL8_008400	10
#define COL8_848400	11
#define COL8_000084	12
#define COL8_840084	13
#define COL8_008484	14
#define COL8_848484	15

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill(char *vram, int xsize, char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int xsize, int ysize);
void putfont(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts(char *vram, int xsize, char *fonts, int x, int y, char c, const char *s);
void init_mouse_cursor(char *mouse, char bc);
void putblock(char *vram, int xsize, int pxsize, int pysize, int px0, int py0, char *buf);

/* dsctbl.c */
#define ADDR_IDT	0x0026f800
#define LIMIT_IDT	0x000007ff
#define ADDR_GDT	0x00270000
#define LIMIT_GDT	0x0000ffff
#define ADDR_BOOTPACK	0x00280000
#define LIMIT_BOOTPACK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e

struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

/* int.h */
#define PORT_PIC0	0x20
#define PORT_PIC0_COMM	(PORT_PIC0)
#define PORT_PIC0_DATA	(PORT_PIC0 + 1)
#define PORT_PIC1	0xa0
#define PORT_PIC1_COMM	(PORT_PIC1)
#define PORT_PIC1_DATA	(PORT_PIC1 + 1)

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level trigggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0c		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

#define PIC_EOI		0x60		/* Specific End Of Interrupt (EOI) */
#define PIC0_EOI_KEY	(PIC_EOI + 1)
#define PIC0_EOI_PIC1	(PIC_EOI + 2)
#define PIC1_EOI_MOUSE	(PIC_EOI + 4)

void init_pic(void);

/* keyboard.c */
#define PORT_KBC_DATA		0x60
#define PORT_KBC_STAT		0x64
#define PORT_KBC_COMM		0x64

#define KBC_COMM_WRITE_CONFIG	0x60
#define KBC_COMM_TO_MOUSE	0xd4
#define KBC_CONFIG_BYTE		0x47
#define KBC_STAT_SEND_NOTREADY	0x02

void inthandler21(int *esp);
void wait_kbc_sendready(void);
void init_keyboard(void);

extern struct FIFO8 keyfifo;

/* mouse.c */
#define MOUSE_COMM_ENABLE	0xf4

struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};

void inthandler2c(int *esp);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data);

extern struct FIFO8 mousefifo;

/* memory.c */
#define MEMMAN_FREES	4094 /* 4096 - 2 */
#define MEMMAN_ADDR	0x003c0000

struct FREEINFO {
	unsigned int addr, size;
};

struct MEMMAN {
	unsigned int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
unsigned int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
unsigned int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);
