
all: download/5.02.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/5.02.tar.gz
	cd temp/sl-5.02 && $(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang -O -Wall -I$(shell pwd)/../sysroot/include/ncursesw -I$(shell pwd)/../sysroot/include -L$(shell pwd)/../sysroot/lib -o sl sl.c -lncursesw -ltinfow
	mkdir -p build/usr/local/bin && cp -rfv temp/sl-5.02/sl build/usr/local/bin/sl
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot

download/5.02.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/mtoyoda/sl/archive/refs/tags/5.02.tar.gz
