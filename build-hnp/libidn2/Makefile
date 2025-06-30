all: download/libidn2-2.3.8.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/libidn2-2.3.8.tar.gz
	cd temp/libidn2-2.3.8 && PKG_CONFIG=/usr/bin/false ./configure --prefix=/data/app/base.org/base_1.0 --host aarch64-unknown-linux-musl --enable-shared --disable-static CC=$(OHOS_SDK_HOME)/native/llvm/bin/aarch64-unknown-linux-ohos-clang LD=$(OHOS_SDK_HOME)/native/llvm/bin/ld.lld AR=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ar RANLIB=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ranlib STRIP=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip --with-libunistring-prefix=$(shell pwd)/../sysroot
	cd temp/libidn2-2.3.8 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -v build/data/app/base.org/base_1.0/lib/*.la
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/libidn2-2.3.8.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/libidn/libidn2-2.3.8.tar.gz
