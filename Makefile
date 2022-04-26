.PHONY: img
img:
	make -r haribote.img

%.o: %.asm
	nasm -o $@ -l $(<:.asm=.lst) $<

nasmfunc.o: nasmfunc.asm
	nasm -f elf32 -o nasmfunc.o -l nasmfunc.lst nasmfunc.asm

bootpack.o: haribote.ld bootpack.c nasmfunc.o mystdio.c
	i386-elf-gcc -march=i486 -m32 -nostdlib -Wall -T haribote.ld -o bootpack.o bootpack.c nasmfunc.o mystdio.c

haribote.sys: asmhead.o bootpack.o
	cat asmhead.o bootpack.o > haribote.sys

haribote.img: ipl.o haribote.sys
	mformat -f 1440 -C -B ipl.o -i haribote.img
	mcopy -i haribote.img haribote.sys ::haribote.sys

.PHONY: run
run:
	make -r img
	qemu-system-i386 -drive file=haribote.img,if=floppy,format=raw -boot a

.PHONY: clean
clean:
	rm -f *.o *.lst *.img *.sys
