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
void load_tr(int tr);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler2c(void);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void taskswitch3(void);
void taskswitch4(void);

/* fifo.c */
#define TIMER_BASE	0
#define KEY_BASE	256
#define MOUSE_BASE	512

struct FIFO32 {
	int *buf;
	int p, q, size, free, flags;
};

void fifo32_init(struct FIFO32 *fifo, int size, int *buf);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

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
#define AR_TSS32	0x0089
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
#define PIC0_EOI_TIMER	(PIC_EOI)
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
void init_keyboard(struct FIFO32 *fifo, int base);

/* mouse.c */
#define MOUSE_COMM_ENABLE	0xf4

struct MOUSE_DEC {
	unsigned char buf[3], phase;
	int x, y, btn;
};

void inthandler2c(int *esp);
void enable_mouse(struct FIFO32 *fifo, int base, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data);

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

/* sheet.c */
#define MAX_SHEETS	256

struct SHEET {
	char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	struct SHTCTL *ctl;
};

struct SHTCTL {
	char *vram, *map;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};

struct SHTCTL *shtctl_init(struct MEMMAN *memman, char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);

/* timer.c */
#define PORT_PIT_CNT0		0x40
#define PORT_PIT_CNT1		0x41
#define PORT_PIT_CNT2		0x42
#define PORT_PIT_COMM		0x43

#define PIT_CHANNEL0		0b00000000
#define PIT_CHANNEL1		0b01000000
#define PIT_CHANNLE2		0b10000000
#define PIT_WRITEBACK		0b11000000

#define PIT_LATCH		0b00000000
#define PIT_LO			0b00010000
#define PIT_HI			0b00100000
#define PIT_LOHI		0b00110000

#define PIT_INTERRUPT		0b00000000
#define PIT_ONESHOT		0b00000010
#define PIT_RATEGEN		0b00000100
#define PIT_SQUARE		0b00000110
#define PIT_SOFT		0b00001000
#define PIT_HARD		0b00001010

#define PIT_BINARY		0b00000000
#define PIT_BCD			0b00000001

#define MAX_TIMERS		500
#define TIMER_FLAG_UNUSE	0
#define TIMER_FLAG_ALLOC	1
#define TIMER_FLAG_USING	2

struct TIMER {
	struct TIMER *next;
	unsigned int timeout;
	unsigned char flag;
	struct FIFO32 *fifo;
	int data;
};

struct TIMERCTL {
	unsigned int count, next;
	struct TIMER *t0;
	struct TIMER timers0[MAX_TIMERS];
};

void init_pit(void);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
void inthandler20(int *esp);

extern struct TIMERCTL timerctl;