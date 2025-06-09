all: download/0.11.1.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/0.11.1.tar.gz
	cd temp/yyjson-0.11.1 && mkdir build
	cd temp/yyjson-0.11.1/build && PKG_CONFIG=/usr/bin/false cmake ../ -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DBUILD_SHARED_LIBS=OFF -DCMAKE_C_COMPILER=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang  -DCMAKE_BUILD_TYPE=RelWithDebInfo 
	cd temp/yyjson-0.11.1/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot

download/0.11.1.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/ibireme/yyjson/archive/refs/tags/0.11.1.tar.gz