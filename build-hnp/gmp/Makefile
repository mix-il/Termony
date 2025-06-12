all: download/gmp-6.3.0.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/gmp-6.3.0.tar.xz
	cd temp/gmp-6.3.0 && mkdir build
	cd temp/gmp-6.3.0/build && ../configure --disable-static --enable-shared --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip
	cd temp/gmp-6.3.0/build && sed -i.bak "s/archive_cmds=\"\"/archive_cmds='\$$CC -shared \$$pic_flag \$$libobjs \$$deplibs \$$compiler_flags \$$wl-soname \$$wl\$$soname -o \$$lib'/" libtool
	cd temp/gmp-6.3.0/build && sed -i.bak "s/build_libtool_libs=no/build_libtool_libs=yes/" libtool
	cd temp/gmp-6.3.0/build && sed -i.bak "s/build_old_libs=yes/build_old_libs=no/" libtool
	cd temp/gmp-6.3.0/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -v build/usr/local/lib/*.la
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/gmp-6.3.0.tar.xz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/gmp/gmp-6.3.0.tar.xz
