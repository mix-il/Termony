all: download/v9.1.1436.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/v9.1.1436.tar.gz
	cd temp/vim-9.1.1436 && ./configure vim_cv_uname_output=Linux --host aarch64-unknown-linux-musl --with-tlib=ncursesw CC=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang LDFLAGS="-L $(shell pwd)/../sysroot/lib"
	cd temp/vim-9.1.1436 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-strip build/usr/local/bin/vim
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot

download/v9.1.1436.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/vim/vim/archive/refs/tags/v9.1.1436.tar.gz