all: download/binutils-2.44.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/binutils-2.44.tar.xz
	cd temp/binutils-2.44 && mkdir build
	cd temp/binutils-2.44/build && ../configure --disable-static --enable-shared --without-zstd --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang LD=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/ld.lld AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip
	cd temp/binutils-2.44/build && make -j $(shell nproc)
	cd temp/binutils-2.44/build && make install DESTDIR=$(shell pwd)/build
	$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip build/usr/local/bin/*
	$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip build/usr/local/lib/*.so
	$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip build/usr/local/aarch64-unknown-linux-musl/bin/*
	mkdir -p ../sysroot
	rm -v build/usr/local/lib/*.la
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/binutils-2.44.tar.xz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/binutils/binutils-2.44.tar.xz
