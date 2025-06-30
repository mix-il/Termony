all: download/libevent-2.1.12-stable.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/libevent-2.1.12-stable.tar.gz
	cd temp/libevent-2.1.12-stable && mkdir build
	cd temp/libevent-2.1.12-stable/build && ../configure --prefix=/data/app/base.org/base_1.0 --disable-static --enable-shared --disable-openssl --host aarch64-unknown-linux-musl CC=$(OHOS_SDK_HOME)/native/llvm/bin/aarch64-unknown-linux-ohos-clang LD=$(OHOS_SDK_HOME)/native/llvm/bin/ld.lld AR=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ar RANLIB=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ranlib STRIP=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip
	cd temp/libevent-2.1.12-stable/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -v build/data/app/base.org/base_1.0/lib/*.la
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/libevent-2.1.12-stable.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz
