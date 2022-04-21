# Haribote OS on macOS
## Motivation
This repository aims to development Haribote OS introduced in [「30日でできる！OS自作入門」](https://book.mynavi.jp/ec/products/detail/id=22078) by using only publically available tools on macOS (after Catalina).

Original Haribote OS is developed by using customized tools called [z_tools](https://github.com/HariboteOS/z_tools_osx) and existing repositories also use them.  
Unfortunately, since some tools are 32-bit applications, we cannot use them on macOS after Catalina.

For example, when use nask (nasm-like assembler), the following error is dispalyed.
```
Bad CPU type in executable
```


## Required Tools
- [QEMU](https://www.qemu.org/): an emulator to boot Haribote OS on virtual environment
- [GNU Make](https://www.gnu.org/software/make/): a tool to compile source codes based on recipe file (Makefile)
- [NASM](https://www.nasm.us/): an assembler (instead of nasm in z_tools)
- [mtools](https://www.gnu.org/software/mtools/): a tool to create image file for floppy disks (instead of edimg in z_tools)
- [i386-elf-toolchain (i386-elf-gcc, i386-elf-binutils)](https://github.com/nativeos/homebrew-i386-elf-toolchain): a C compiler (instead of cc1 in z_tools)

### How to install
```
brew install qemu make nasm mtools
brew tap nativeos/i386-elf-toolchain
brew install i386-elf-binutils i386-elf-gcc
```

If versions are displayed by the following commands, these are installed successfully.
```
qemu-system-i386 --version
make --version
nasm --version
mtools --version
i386-elf-gcc --version
```

### Operation Verfied Environment
- macOS Monterey 12.3.1
- QEMU emulator version 6.1.0
- GNU Make 3.81
- NASM version 2.15.05
- mtools (GNU mtools) 4.0.31
- i386-elf-gcc (GCC) 11.1.0


## References
- [tools/nask - hrb-wiki](http://hrb.osask.jp/wiki/?tools/nask)
- [『30日でできる！OS自作入門』のメモ](https://vanya.jp.net/os/haribote.html#hrb)
- [『30日でできる！OS自作入門』を macOS Catalina で実行する - Qiita](https://qiita.com/noanoa07/items/8828c37c2e286522c7ee)
- [noanoa07/myHariboteOS: 『30日でできる！OS自作入門』 for macOS Catalina](https://github.com/noanoa07/myHariboteOS)
- [sprintfを実装する | OS自作入門 5日目-2 【Linux】 | サラリーマンがハッカーを真剣に目指す](http://bttb.s1.valueserver.jp/wordpress/blog/2017/12/17/makeos-5-2/)

