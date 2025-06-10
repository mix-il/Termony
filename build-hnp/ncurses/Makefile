# use tic from homebrew
export PATH := /opt/homebrew/opt/ncurses/bin/:$(PATH)

all: download/ncurses-6.5.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/ncurses-6.5.tar.gz
	cd temp/ncurses-6.5 && mkdir build
	cd temp/ncurses-6.5/build && ../configure --with-strip-program="$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip" --without-ada --disable-mixed-case --prefix=/data/app/base.org/base_1.0 --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang CXX=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang
	cd temp/ncurses-6.5/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot

download/ncurses-6.5.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/ncurses/ncurses-6.5.tar.gz
