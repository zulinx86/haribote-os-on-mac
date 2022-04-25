.PHONY: img
img:
	make -r haribote.img

ipl.o: ipl.asm
	nasm -o ipl.o ipl.asm

haribote.o: haribote.asm
	nasm -o haribote.o haribote.asm

haribote.img: ipl.o haribote.o
	mformat -f 1440 -C -B ipl.o -i haribote.img
	mcopy -i haribote.img haribote.o ::haribote.o

.PHONY: run
run:
	make -r img
	qemu-system-i386 -drive file=haribote.img,if=floppy,format=raw -boot a

.PHONY: clean
clean:
	rm -f *.o *.lst *.img
