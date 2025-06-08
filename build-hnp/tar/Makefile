all: download/tar-1.35.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/tar-1.35.tar.xz
	cd temp/tar-1.35 && mkdir build
	cd temp/tar-1.35/build && ../configure --host aarch64-unknown-linux-musl CC=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-strip
	cd temp/tar-1.35/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rv ./build/usr/local/. ../sysroot

download/tar-1.35.tar.xz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/tar/tar-1.35.tar.xz