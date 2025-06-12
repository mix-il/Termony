all: download/libevent-2.1.12-stable.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/libevent-2.1.12-stable.tar.gz
	cd temp/libevent-2.1.12-stable && mkdir build
	cd temp/libevent-2.1.12-stable/build && ../configure --disable-static --enable-shared --disable-openssl --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip
	# fix libtool compat with clang, see https://github.com/libexpat/libexpat/issues/312
	cd temp/libevent-2.1.12-stable/build && sed -i.bak "s/archive_cmds=\"\"/archive_cmds='\$$CC -shared \$$pic_flag \$$libobjs \$$deplibs \$$compiler_flags \$$wl-soname \$$wl\$$soname -o \$$lib'/" libtool
	cd temp/libevent-2.1.12-stable/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -v build/usr/local/lib/*.la
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/libevent-2.1.12-stable.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz
