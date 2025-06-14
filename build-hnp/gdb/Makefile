include ../utils/Makefrag

all: download/gdb-16.3.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/gdb-16.3.tar.xz
	cd temp/gdb-16.3 && mkdir build
	cd temp/gdb-16.3/build && ../configure --with-lzma --disable-sim --disable-static --enable-shared --with-system-readline --host aarch64-unknown-linux-musl
	cd temp/gdb-16.3/build && make -j $(shell nproc)
	cd temp/gdb-16.3/build && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -v build/usr/local/lib/*.la
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/gdb-16.3.tar.xz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/gdb/gdb-16.3.tar.xz
