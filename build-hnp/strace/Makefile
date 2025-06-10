all: download/strace-6.15.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/strace-6.15.tar.xz
	cd temp/strace-6.15 && mkdir build
	cd temp/strace-6.15/build && ../configure --enable-mpers=no --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip CFLAGS="-Wno-error"
	cd temp/strace-6.15/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot

download/strace-6.15.tar.xz:
	mkdir -p download
	cd download && wget https://github.com/strace/strace/releases/download/v6.15/strace-6.15.tar.xz
