all: download/strace-6.15.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/strace-6.15.tar.xz
	cd temp/strace-6.15 && mkdir build
	cd temp/strace-6.15/build && ../configure --enable-mpers=no --host aarch64-unknown-linux-musl CC=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-strip CFLAGS="-Wno-error"
	cd temp/strace-6.15/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot

download/strace-6.15.tar.xz:
	mkdir -p download
	cd download && wget https://github.com/strace/strace/releases/download/v6.15/strace-6.15.tar.xz