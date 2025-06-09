

all: download/htop-3.4.1.tar.xz
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && tar xvf ../download/htop-3.4.1.tar.xz
	cd temp/htop-3.4.1 && PKG_CONFIG=/usr/bin/false ./configure --host aarch64-unknown-linux-musl --enable-affinity --enable-static CC=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-strip CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" LDFLAGS="-L$(shell pwd)/../sysroot/lib"
	cd temp/htop-3.4.1 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rv ./build/usr/local/. ../sysroot

download/htop-3.4.1.tar.xz:
	mkdir -p download
	cd download && wget https://github.com/htop-dev/htop/releases/download/3.4.1/htop-3.4.1.tar.xz