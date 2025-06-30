include ../utils/Makefrag

all: download/gmp-6.3.0.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/gmp-6.3.0.tar.xz
	cd temp/gmp-6.3.0 && mkdir build
	cd temp/gmp-6.3.0/build && ../configure --prefix=/data/app/base.org/base_1.0 --disable-static --enable-shared --host aarch64-unknown-linux-musl
	cd temp/gmp-6.3.0/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -v build/data/app/base.org/base_1.0/lib/*.la
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/gmp-6.3.0.tar.xz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/gmp/gmp-6.3.0.tar.xz
