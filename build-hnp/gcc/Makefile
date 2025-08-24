include ../utils/Makefrag

all: download/gcc-15.1.0.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/gcc-15.1.0.tar.xz
	cd temp/gcc-15.1.0 && mkdir build
	cd temp/gcc-15.1.0/build && ../configure --host $(OHOS_ARCH)-unknown-linux-musl --enable-host-shared
	cd temp/gcc-15.1.0/build && make all-gcc -j $(shell nproc) -k || true
	cd temp/gcc-15.1.0/build && cat ../../../0001-fix-gcc.diff | patch -Np1
	cd temp/gcc-15.1.0/build && make all-gcc -j $(shell nproc)
	cd temp/gcc-15.1.0/build && make install-gcc DESTDIR=$(shell pwd)/build
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/usr/local/bin/*
	rm -f build/usr/local/libexec/gcc/$(OHOS_ARCH)-unknown-linux-musl/15.1.0/*.la
	find build/usr/local/libexec/gcc/$(OHOS_ARCH)-unknown-linux-musl/15.1.0/ -maxdepth 1 -type f | xargs $(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/gcc-15.1.0.tar.xz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/gcc/gcc-15.1.0/gcc-15.1.0.tar.xz
