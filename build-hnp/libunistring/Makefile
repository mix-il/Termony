all: download/libunistring-1.3.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/libunistring-1.3.tar.xz
	cd temp/libunistring-1.3 && PKG_CONFIG=/usr/bin/false LIBTOOL=glibtool LIBTOOLIZE=glibtoolize ./configure --host aarch64-unknown-linux-musl --enable-shared --disable-static CC=$(OHOS_SDK_HOME)/native/llvm/bin/aarch64-unknown-linux-ohos-clang LD=$(OHOS_SDK_HOME)/native/llvm/bin/ld.lld AR=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ar RANLIB=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ranlib STRIP=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip
	cd temp/libunistring-1.3 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -v build/usr/local/lib/*.la
	$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip build/usr/local/lib/*.so
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/libunistring-1.3.tar.xz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/libunistring/libunistring-1.3.tar.xz
