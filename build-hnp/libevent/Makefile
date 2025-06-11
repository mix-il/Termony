all: download/libevent-2.1.12-stable.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/libevent-2.1.12-stable.tar.gz
	cd temp/libevent-2.1.12-stable && mkdir build
	cd temp/libevent-2.1.12-stable/build && ../configure --enable-static --disable-shared --disable-openssl --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip
	cd temp/libevent-2.1.12-stable/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot

download/libevent-2.1.12-stable.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz
