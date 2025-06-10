all: download/libunistring-1.3.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/libunistring-1.3.tar.xz
	cd temp/libunistring-1.3 && PKG_CONFIG=/usr/bin/false LIBTOOL=glibtool LIBTOOLIZE=glibtoolize ./configure --host aarch64-unknown-linux-musl --disable-shared --enable-static CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip
	cd temp/libunistring-1.3 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot

download/libunistring-1.3.tar.xz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/libunistring/libunistring-1.3.tar.xz
