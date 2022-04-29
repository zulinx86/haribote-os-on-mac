OBJS_BOOTPACK = bootpack.o nasmfunc.o mystdio.o graphic.o dsctbl.o int.o fifo.o keyboard.o mouse.o
HDRS_BOOTPACK = mystdio.h bootpack.h

.PHONY: img
img:
	make -r haribote.img

%.bin: %.asm
	nasm -o $@ -l $(<:.asm=.lst) $<

nasmfunc.o: nasmfunc.asm
	nasm -f elf32 -o nasmfunc.o -l nasmfunc.lst nasmfunc.asm

%.o: %.c
	i386-elf-gcc -c -march=i486 -m32 -nostdlib -Wall -o $@ $<

bootpack.hrb: haribote.ld $(OBJS_BOOTPACK) $(HDRS_BOOTPACK)
	i386-elf-gcc -march=i486 -m32 -nostdlib -Wall -T haribote.ld -o $@ $(OBJS_BOOTPACK) 

haribote.sys: asmhead.bin bootpack.hrb
	cat $^ > $@

haribote.img: ipl.bin haribote.sys
	mformat -f 1440 -C -B ipl.bin -i $@
	mcopy -i $@ haribote.sys ::haribote.sys

.PHONY: run
run:
	make -r img
	qemu-system-i386 -drive file=haribote.img,if=floppy,format=raw -boot a

.PHONY: clean
clean:
	rm -f *.bin *.lst *.o *.hrb *.sys *.img
