all: download/expat-2.7.1.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/expat-2.7.1.tar.xz
	cd temp/expat-2.7.1 && mkdir build
	cd temp/expat-2.7.1/build && ../configure --disable-static --enable-shared --without-zstd --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip
	# fix libtool compat with clang, see https://github.com/libexpat/libexpat/issues/312
	cd temp/expat-2.7.1/build && sed -i.bak "s/archive_cmds=\"\"/archive_cmds='\$$CC -shared \$$pic_flag \$$libobjs \$$deplibs \$$compiler_flags \$$wl-soname \$$wl\$$soname -o \$$lib'/" libtool
	cd temp/expat-2.7.1/build && sed -i.bak "s/build_libtool_libs=no/build_libtool_libs=yes/" libtool
	cd temp/expat-2.7.1/build && sed -i.bak "s/build_old_libs=yes/build_old_libs=no/" libtool
	cd temp/expat-2.7.1/build && make -j $(shell nproc)
	cd temp/expat-2.7.1/build && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -v build/usr/local/lib/*.la
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/expat-2.7.1.tar.xz:
	mkdir -p download
	cd download && wget https://github.com/libexpat/libexpat/releases/download/R_2_7_1/expat-2.7.1.tar.xz
