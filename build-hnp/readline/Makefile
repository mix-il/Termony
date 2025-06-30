all: download/readline-8.2.tar.gz
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && tar xvf ../download/readline-8.2.tar.gz
	cd temp/readline-8.2 && PKG_CONFIG=/usr/bin/false ./configure --prefix=/data/app/base.org/base_1.0 --host aarch64-unknown-linux-musl CC=$(OHOS_SDK_HOME)/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ar RANLIB=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ranlib STRIP=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" LDFLAGS="-L$(shell pwd)/../sysroot/lib"
	cd temp/readline-8.2 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/readline-8.2.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/readline/readline-8.2.tar.gz
