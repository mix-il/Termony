include ../utils/Makefrag

# required by mkbuiltin cross-build process when using gcc 13+
export CFLAGS_FOR_BUILD := -std=gnu17

all: download/bash-5.2.37.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/bash-5.2.37.tar.gz
	cd temp/bash-5.2.37 && mkdir build
	cd temp/bash-5.2.37/build && ../configure --prefix=/data/app/base.org/base_1.0 --host aarch64-unknown-linux-musl --without-bash-malloc
	cd temp/bash-5.2.37/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/bash-5.2.37.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/bash/bash-5.2.37.tar.gz
