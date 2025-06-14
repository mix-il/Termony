include ../utils/Makefrag

all: download/ncurses-6.5.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/ncurses-6.5.tar.gz
	cd temp/ncurses-6.5 && mkdir build
	cd temp/ncurses-6.5/build && ../configure --with-shared --with-strip-program="$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip" --with-termlib --with-cxx-binding --with-widec --without-ada --disable-mixed-case --prefix=/data/app/base.org/base_1.0 --host aarch64-unknown-linux-musl
	cd temp/ncurses-6.5/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -f build/data/app/base.org/base_1.0/lib/*.a
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/ncurses-6.5.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/ncurses/ncurses-6.5.tar.gz
