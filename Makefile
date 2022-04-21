.PHONY: img
img:
	make -r helloos.img

helloos.img: helloos.asm
	nasm -o helloos.img helloos.asm

.PHONY: run
run:
	make -r img
	qemu-system-i386 -drive file=helloos.img,if=floppy,format=raw -boot a

.PHONY: clean
clean:
	rm -f helloos.img
