all: download/2.45.0.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/2.45.0.tar.gz
	cd temp/fastfetch-2.45.0 && mkdir build && /usr/bin/sed -i.bak 's|FF_HAVE_UTMPX=1|FF_HAVE_UTMPX=0|g' CMakeLists.txt
	cd temp/fastfetch-2.45.0/build && PKG_CONFIG=/usr/bin/false cmake ../ -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DIS_MUSL=ON -DENABLE_PCIACCESS=ON -DENABLE_SQLITE3=OFF -DENABLE_VULKAN=ON -DCMAKE_C_COMPILER=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang -DCMAKE_CXX_COMPILER=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_SYSROOT=$(TOOL_HOME)/sdk/default/openharmony/native/sysroot
	cd temp/fastfetch-2.45.0/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/2.45.0.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/fastfetch-cli/fastfetch/archive/refs/tags/2.45.0.tar.gz
