all: download/xz-5.8.1.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/xz-5.8.1.tar.xz
	cd temp/xz-5.8.1 && mkdir build
	cd temp/xz-5.8.1/build && ../configure --disable-static --enable-shared --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang LD=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/ld.lld AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip
	cd temp/xz-5.8.1/build && make -j $(shell nproc)
	cd temp/xz-5.8.1/build && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -v build/usr/local/lib/*.la
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/xz-5.8.1.tar.xz:
	mkdir -p download
	cd download && wget https://github.com/tukaani-project/xz/releases/download/v5.8.1/xz-5.8.1.tar.xz
