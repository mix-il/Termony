include ../utils/Makefrag

all: download/elf
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && cp -vr ../download/elf elf
	cd temp/elf && git apply ../../jit.patch
	cd temp/elf && make -C src ARCH=aarch64 CFLAGS="-ffreestanding -DELFCLASS=ELFCLASS64"
	cp temp/elf/src/loader ./build/bin/
	mkdir -p ../sysroot
	cp -rv ./build/. ../sysroot | tee file.lst

download/elf:
	mkdir -p download
	cd download && rm -rf elf && git clone https://github.com/MikhailProg/elf
