all: download/gcc-15.1.0.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/gcc-15.1.0.tar.xz
	cd temp/gcc-15.1.0 && mkdir build
	cd temp/gcc-15.1.0/build && ../configure --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang CXX=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang++ AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip CXXFLAGS="-I$(shell pwd)/../sysroot/include" CFLAGS="-I$(shell pwd)/../sysroot/include" LDFLAGS="-L$(shell pwd)/../sysroot/lib" --enable-host-shared
	# still some problems unresolved
	cd temp/gcc-15.1.0/build && make all-gcc -j $(shell nproc) -k || true
	cd temp/gcc-15.1.0/build && make install-gcc DESTDIR=$(shell pwd)/build || true
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/gcc-15.1.0.tar.xz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/gcc/gcc-15.1.0/gcc-15.1.0.tar.xz
