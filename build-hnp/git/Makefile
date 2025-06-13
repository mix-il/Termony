all: download/v2.49.0.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/v2.49.0.tar.gz
	cd temp/git-2.49.0 && mkdir build
	cd temp/git-2.49.0 && make -j $(shell nproc) uname_S=Linux NO_REGEX=NeedsStartEnd CFLAGS="-DNO_PTHREADS -DNO_GETTEXT -Wno-int-conversion -I$(shell pwd)/../sysroot/include" LDFLAGS="-L $(shell pwd)/../sysroot/lib" V=1 prefix=/data/app/base.org/base_1.0 CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip LD=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang all install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	#$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/bin/*
	#$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/libexec/git-core/*
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/v2.49.0.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/git/git/archive/refs/tags/v2.49.0.tar.gz
