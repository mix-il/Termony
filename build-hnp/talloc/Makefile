include ../utils/Makefrag

all: download/talloc-2.4.3.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/talloc-2.4.3.tar.gz
	cd temp/talloc-2.4.3 && mkdir build
	cd temp/talloc-2.4.3 && \
	echo "Checking uname sysname type: aarch64\n\
Checking uname machine type: aarch64-unknown-linux-musl\n\
Checking uname release type: release\n\
Checking uname version type: 5.2\n\
rpath library support: OK\n\
Checking getconf LFS_CFLAGS: "-D_LARGEFILE64_SOURCE=1"\n\
Checking for -D_FILE_OFFSET_BITS=64: OK\n\
Checking for -D_LARGE_FILES: OK\n\
Checking for HAVE_SECURE_MKSTEMP: OK\n\
-Wl,--version-script support: FAIL\n\
Checking for large file support without additional flags: OK\n\
Checking correct behavior of strtoll: OK\n\
Checking for working strptime: OK\n\
Checking for C99 vsnprintf: OK\n\
Checking for HAVE_SHARED_MMAP: OK\n\
Checking for HAVE_MREMAP: OK\n\
Checking for HAVE_INCOHERENT_MMAP: OK" >> aarch64-unknown-linux-musl.txt && \
    CFLAGS="--sysroot=$(TOOL_HOME)/sdk/default/openharmony/native/sysroot -O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer -I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" \
    LDFLAGS="--sysroot=$(TOOL_HOME)/sdk/default/openharmony/native/sysroot -L$(TOOL_HOME)/sdk/default/openharmony/native/sysroot/usr/lib" \
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
	