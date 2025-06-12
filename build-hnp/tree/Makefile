all: download/2.2.1.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/2.2.1.tar.gz
	cd temp/tree-2.2.1 && make CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang CFLAGS="-O3 -static -std=c11 -pedantic -Wall -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DLINUX" LDFLAGS="-static"
	cd temp/tree-2.2.1 && make install MANDIR=$(shell pwd)/build/share/man DESTDIR=$(shell pwd)/build/bin
	mkdir -p ../sysroot
	cp -rv ./build/. ../sysroot | tee file.lst

download/2.2.1.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/Old-Man-Programmer/tree/archive/refs/tags/2.2.1.tar.gz
