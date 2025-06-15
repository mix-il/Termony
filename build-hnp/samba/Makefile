all: download/samba-4.22.2.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/samba-4.22.2.tar.gz
	cd temp/samba-4.22.2 && mkdir build
	cd temp/samba-4.22.2 && \
	echo "Checking uname sysname type: aarch64\n\
Checking uname machine type: aarch64-unknown-linux-musl\n\
Checking uname release type: release\n\
Checking uname version type: 5.2\n\
rpath library support: OK\n\
Checking getconf LFS_CFLAGS: "-D_LARGEFILE64_SOURCE=1"\n\
Checking for -D_FILE_OFFSET_BITS=64: OK\n\
Checking for -D_LARGE_FILES: OK\n\
Checking for HAVE_SECURE_MKSTEMP: OK\n\
Checking for statvfs (SVR4): OK\n\
Checking for program \'perl\': NO" >> aarch64-unknown-linux-musl.txt && \
    CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang \
    LD=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/ld.lld \
    AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar \
    RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib \
    STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip \
    CFLAGS="--sysroot=$(TOOL_HOME)/sdk/default/openharmony/native/sysroot -O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer -I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" \
    LDFLAGS="--sysroot=$(TOOL_HOME)/sdk/default/openharmony/native/sysroot -L$(TOOL_HOME)/sdk/default/openharmony/native/sysroot/usr/lib" \
	PATH="$(shell env)/build_helpers:$$PATH" \
	./configure \
	--prefix=/data/app/base.org/base_1.0 \
	--cross-compile --host aarch64-unknown-linux-musl  --cross-answers=aarch64-unknown-linux-musl.txt \
	--without-pam \
	--disable-python --without-ad-dc \
	--without-ldap \
	--without-libunwind
	cd temp/samba-4.22.2 && make JOBS=$(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/samba-4.22.2.tar.gz:
	mkdir -p download
	cd download && wget https://download.samba.org/pub/samba/stable/samba-4.22.2.tar.gz
