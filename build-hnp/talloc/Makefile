include ../utils/Makefrag

all: download/talloc-2.4.3.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/talloc-2.4.3.tar.gz
	cd temp/talloc-2.4.3 && mkdir build
	cp aarch64-unknown-linux-musl.txt temp/talloc-2.4.3
	cd temp/talloc-2.4.3 && \
    CFLAGS="--sysroot=$(OHOS_SDK_HOME)/native/sysroot -O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer -I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" \
    LDFLAGS="--sysroot=$(OHOS_SDK_HOME)/native/sysroot -L$(OHOS_SDK_HOME)/native/sysroot/usr/lib" \
	./configure  --prefix=/data/app/base.org/base_1.0 \
	--cross-compile --host aarch64-unknown-linux-musl --cross-answers=aarch64-unknown-linux-musl.txt \
	--disable-python \
	--check-c-compiler=clang
	cd temp/talloc-2.4.3 && make -j$(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/talloc-2.4.3.tar.gz:
	mkdir -p download
	cd download && wget https://www.samba.org/ftp/talloc/talloc-2.4.3.tar.gz
