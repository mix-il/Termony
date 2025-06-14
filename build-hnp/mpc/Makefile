all: download/mpc-1.3.1.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/mpc-1.3.1.tar.gz
	cd temp/mpc-1.3.1 && mkdir build
	cd temp/mpc-1.3.1/build && ../configure --disable-static --enable-shared --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang LD=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/ld.lld AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip CFLAGS="-I$(shell pwd)/../sysroot/include" LDFLAGS="-L$(shell pwd)/../sysroot/lib"
	cd temp/mpc-1.3.1/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -v build/usr/local/lib/*.la
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/mpc-1.3.1.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/mpc/mpc-1.3.1.tar.gz
