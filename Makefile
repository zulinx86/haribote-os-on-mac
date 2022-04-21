.PHONY: img
img:
	make -r helloos.img

ipl.bin: ipl.asm
	nasm -o ipl.bin ipl.asm

helloos.img: ipl.bin
	mformat -f 1440 -C -B ipl.bin -i helloos.img

.PHONY: run
run:
	make -r img
	qemu-system-i386 -drive file=helloos.img,if=floppy,format=raw -boot a

.PHONY: clean
clean:
	rm -f *.bin *.lst *.img
