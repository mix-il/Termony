all: download/bash-5.2.37.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/bash-5.2.37.tar.gz
	cd temp/bash-5.2.37 && mkdir build
	cd temp/bash-5.2.37/build && ../configure --host aarch64-unknown-linux-musl --without-bash-malloc CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang
	cd temp/bash-5.2.37/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot

download/bash-5.2.37.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/bash/bash-5.2.37.tar.gz
