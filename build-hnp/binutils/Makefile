all: download/binutils-2.44.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/binutils-2.44.tar.xz
	cd temp/binutils-2.44 && mkdir build
	cd temp/binutils-2.44/build && ../configure --disable-static --enable-shared --without-zstd --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip
	cd temp/binutils-2.44/build && make -j $(shell nproc)
	# fix libtool compat with clang, see https://github.com/libexpat/libexpat/issues/312
	cd temp/binutils-2.44/build/bfd && sed -i.bak "s/archive_cmds=\"\"/archive_cmds='\$$CC -shared \$$pic_flag \$$libobjs \$$deplibs \$$compiler_flags \$$wl-soname \$$wl\$$soname -o \$$lib'/" libtool
	cd temp/binutils-2.44/build/bfd && sed -i.bak "s/build_libtool_libs=no/build_libtool_libs=yes/" libtool
	cd temp/binutils-2.44/build/bfd && sed -i.bak "s/build_old_libs=yes/build_old_libs=no/" libtool
	cd temp/binutils-2.44/build/libsframe && sed -i.bak "s/archive_cmds=\"\"/archive_cmds='\$$CC -shared \$$pic_flag \$$libobjs \$$deplibs \$$compiler_flags \$$wl-soname \$$wl\$$soname -o \$$lib'/" libtool
	cd temp/binutils-2.44/build/libsframe && sed -i.bak "s/build_libtool_libs=no/build_libtool_libs=yes/" libtool
	cd temp/binutils-2.44/build/libsframe && sed -i.bak "s/build_old_libs=yes/build_old_libs=no/" libtool
	cd temp/binutils-2.44/build/opcodes && sed -i.bak "s/archive_cmds=\"\"/archive_cmds='\$$CC -shared \$$pic_flag \$$libobjs \$$deplibs \$$compiler_flags \$$wl-soname \$$wl\$$soname -o \$$lib'/" libtool
	cd temp/binutils-2.44/build/opcodes && sed -i.bak "s/build_libtool_libs=no/build_libtool_libs=yes/" libtool
	cd temp/binutils-2.44/build/opcodes && sed -i.bak "s/build_old_libs=yes/build_old_libs=no/" libtool
	cd temp/binutils-2.44/build/libctf && sed -i.bak "s/archive_cmds=\"\"/archive_cmds='\$$CC -shared \$$pic_flag \$$libobjs \$$deplibs \$$compiler_flags \$$wl-soname \$$wl\$$soname -o \$$lib'/" libtool
	cd temp/binutils-2.44/build/libctf && sed -i.bak "s/build_libtool_libs=no/build_libtool_libs=yes/" libtool
	cd temp/binutils-2.44/build/libctf && sed -i.bak "s/build_old_libs=yes/build_old_libs=no/" libtool
	cd temp/binutils-2.44/build && make clean
	cd temp/binutils-2.44/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip build/usr/local/bin/*
	$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip build/usr/local/lib/*.so
	$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip build/usr/local/aarch64-unknown-linux-musl/bin/*
	mkdir -p ../sysroot
	rm -v build/usr/local/lib/*.la
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/binutils-2.44.tar.xz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/binutils/binutils-2.44.tar.xz
