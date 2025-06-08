all: tree.hnp

tree.hnp: download/2.2.1.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/2.2.1.tar.gz
	cd temp/tree-2.2.1 && make CC=/Applications/DevEco-Studio.app/Contents//sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang CFLAGS="-O3 -static -std=c11 -pedantic -Wall -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DLINUX" LDFLAGS="-static" PREFIX=/
	cd temp/tree-2.2.1 && make install MANDIR=$(shell pwd)/build/share/man DESTDIR=$(shell pwd)/build/bin
	/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/toolchains/hnpcli pack -i build -n tree -v 2.2.1

download/2.2.1.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/Old-Man-Programmer/tree/archive/refs/tags/2.2.1.tar.gz