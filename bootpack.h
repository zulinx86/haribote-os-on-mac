/* asmhead.asm */
struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram, *fonts;
};

#define ADDR_BOOTINFO	0x0ff0

/* nasmfunc.asm */
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

/* graphic.c */
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill(char *vram, int xsize, char c, int x0, int y0, int x1, int y1);
void init_screen(char *vram, int xsize, int ysize);
void putfont(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts(char *vram, int xsize, char *fonts, int x, int y, char c, const char *s);
void init_mouse_cursor(char *mouse, char bc);
void putblock(char *vram, int xsize, int pxsize, int pysize, int px0, int py0, char *buf);

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

/* dsctbl.c */
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

#define ADDR_IDT	0x0026f800
#define LIMIT_IDT	0x000007ff
#define ADDR_GDT	0x00270000
#define LIMIT_GDT	0x0000ffff
#define ADDR_BOOTPACK	0x00280000
#define LIMIT_BOOTPACK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a

/* int.h */
void init_pic(void);

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
