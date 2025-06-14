include ../utils/Makefrag

all: download/mpfr-4.2.2.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/mpfr-4.2.2.tar.xz
	cd temp/mpfr-4.2.2 && mkdir build
	cd temp/mpfr-4.2.2/build && ../configure --disable-static --enable-shared --host aarch64-unknown-linux-musl
	cd temp/mpfr-4.2.2/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -v build/usr/local/lib/*.la
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/mpfr-4.2.2.tar.xz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/mpfr/mpfr-4.2.2.tar.xz
