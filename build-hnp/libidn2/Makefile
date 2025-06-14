all: download/libidn2-2.3.8.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/libidn2-2.3.8.tar.gz
	cd temp/libidn2-2.3.8 && PKG_CONFIG=/usr/bin/false ./configure --host aarch64-unknown-linux-musl --enable-shared --disable-static CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang LD=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/ld.lld AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip --with-libunistring-prefix=$(shell pwd)/../sysroot
	cd temp/libidn2-2.3.8 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -v build/usr/local/lib/*.la
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/libidn2-2.3.8.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/libidn/libidn2-2.3.8.tar.gz
