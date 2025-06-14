all: download/aria2-1.37.0.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/aria2-1.37.0.tar.gz
	cd temp/aria2-1.37.0 && mkdir build
	cd temp/aria2-1.37.0 && PKG_CONFIG=/usr/bin/false ./configure --prefix=/data/app/base.org/base_1.0 --host aarch64-unknown-linux-musl --with-openssl --without-gnutls OPENSSL_CFLAGS="-D_FORTIFY_SOURCE=2" OPENSSL_LIBS="-lssl -lcrypto" --with-sysroot=$(TOOL_HOME)/sdk/default/openharmony/native/sysroot CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang CXX=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang++ AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip --enable-libaria2 --with-ca-bundle=/etc/ssl/certs/cacert.pem CFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2 -O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer" CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" LDFLAGS="-L$(shell pwd)/../sysroot/lib"
	cd temp/aria2-1.37.0 && sed -i.bak "s/archive_cmds=\".*\"/archive_cmds='\$$CC -shared \$$pic_flag \$$libobjs \$$deplibs \$$compiler_flags \$$wl-soname \$$wl\$$soname -o \$$lib'/" libtool
	cd temp/aria2-1.37.0 && sed -i.bak "s/build_libtool_libs=no/build_libtool_libs=yes/" libtool
	cd temp/aria2-1.37.0 && sed -i.bak "s/build_old_libs=yes/build_old_libs=no/" libtool
	cd temp/aria2-1.37.0 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	rm -v build/data/app/base.org/base_1.0/lib/*.la
	mkdir -p ../sysroot
	#$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/bin/*
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/aria2-1.37.0.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/aria2/aria2/releases/download/release-1.37.0/aria2-1.37.0.tar.gz
