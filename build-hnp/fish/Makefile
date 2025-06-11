all: download/fish-3.7.1.tar.xz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/fish-3.7.1.tar.xz
	# TCSANOW does not work, use TCSADRAIN
	sed -i.bak 's/TCSANOW,/TCSADRAIN,/' $(wildcard temp/fish-3.7.1/src/*.cpp)
	cd temp/fish-3.7.1 && cat ../../0001-fix-cmake.diff | patch -Np1
	cd temp/fish-3.7.1 && mkdir build
	cd temp/fish-3.7.1/build && PKG_CONFIG=/bin/true cmake ../ -DCURSES_HAVE_CURSES_H=ON -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_C_COMPILER=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang -DCMAKE_CXX_COMPILER=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang++ -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCURSES_FOUND=ON -DCURSES_INCLUDE_PATH="$(shell pwd)/../sysroot/include;$(shell pwd)/../sysroot/include/ncursesw" -DCURSES_LIBRARY=ncursesw -DCMAKE_EXE_LINKER_FLAGS="-L $(shell pwd)/../sysroot/lib" -DCMAKE_CXX_FLAGS="-DTPUTS_USES_INT_ARG=1" -DFISH_USE_SYSTEM_PCRE2=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_INSTALL_PREFIX=/data/app/base.org/base_1.0
	cd temp/fish-3.7.1/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip build/data/app/base.org/base_1.0/bin/*
	mkdir -p ../sysroot
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot
	# it makes hap installation fail
	rm -f ../sysroot/share/fish/completions/..fish

download/fish-3.7.1.tar.xz:
	mkdir -p download
	cd download && wget https://github.com/fish-shell/fish-shell/releases/download/3.7.1/fish-3.7.1.tar.xz
