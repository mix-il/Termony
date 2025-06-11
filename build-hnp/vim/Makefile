all: download/v9.1.1436.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/v9.1.1436.tar.gz
	cd temp/vim-9.1.1436 && cat ../../0001-fix-tcsetattr.diff | patch -Np1
	cd temp/vim-9.1.1436 && cat ../../0002-fix-viminfo-gid-check.diff | patch -Np1
	cd temp/vim-9.1.1436 && ./configure --prefix=/data/app/base.org/base_1.0 vim_cv_uname_output=Linux --host aarch64-unknown-linux-musl --with-tlib=ncursesw CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang LDFLAGS="-L $(shell pwd)/../sysroot/lib"
	cd temp/vim-9.1.1436 && make -j $(shell nproc) && make install STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot

download/v9.1.1436.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/vim/vim/archive/refs/tags/v9.1.1436.tar.gz
