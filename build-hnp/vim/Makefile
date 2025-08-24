include ../utils/Makefrag

all: download/v9.1.1436.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/v9.1.1436.tar.gz
	cd temp/vim-9.1.1436 && cat ../../0001-fix-tcsetattr.diff | patch -Np1
	cd temp/vim-9.1.1436 && cat ../../0002-fix-viminfo-gid-check.diff | patch -Np1
	cd temp/vim-9.1.1436 && ./configure --prefix=$(PREFIX) vim_cv_uname_output=Linux --host $(OHOS_ARCH)-unknown-linux-musl --with-tlib=tinfow
	cd temp/vim-9.1.1436 && make -j $(shell nproc) && make install STRIP=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/v9.1.1436.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/vim/vim/archive/refs/tags/v9.1.1436.tar.gz
