all: download/kbd-2.8.0.tar.xz
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && tar xvf ../download/kbd-2.8.0.tar.xz
	cd temp/kbd-2.8.0 && PKG_CONFIG=/usr/bin/true ./configure ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes --disable-vlock --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip CFLAGS="-Wno-int-conversion"
	cd temp/kbd-2.8.0 && echo "static char *program_invocation_short_name = \"argv[0]\";" >> config.h
	cd temp/kbd-2.8.0 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rv ./build/usr/local/. ../sysroot | tee file.lst

download/kbd-2.8.0.tar.xz:
	mkdir -p download
	cd download && wget https://www.kernel.org/pub/linux/utils/kbd/kbd-2.8.0.tar.xz
