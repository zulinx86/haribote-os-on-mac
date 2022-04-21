.PHONY: run
run:
	qemu-system-i386 -drive file=helloos.img,if=floppy,format=raw -boot a
