all: download/readline-8.2.tar.gz
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && tar xvf ../download/readline-8.2.tar.gz
	cd temp/readline-8.2 && PKG_CONFIG=/usr/bin/false ./configure --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" LDFLAGS="-L$(shell pwd)/../sysroot/lib"
	cd temp/readline-8.2 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rv ./build/usr/local/. ../sysroot

download/readline-8.2.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/readline/readline-8.2.tar.gz