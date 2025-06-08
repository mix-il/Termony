# use tic from homebrew
export PATH := /opt/homebrew/opt/ncurses/bin/:$(PATH)

all: download/ncurses-6.5.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/ncurses-6.5.tar.gz
	cd temp/ncurses-6.5 && mkdir build
	cd temp/ncurses-6.5/build && ../configure --host aarch64-unknown-linux-musl CC=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang CXX=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang
	cd temp/ncurses-6.5/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/. ../sysroot

download/ncurses-6.5.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/ncurses/ncurses-6.5.tar.gz