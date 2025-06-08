all: xxhash.hnp

xxhash.hnp: download/v0.8.3.tar.gz
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && tar xvf ../download/v0.8.3.tar.gz
	cd temp/xxHash-0.8.3 && mkdir build
	cd temp/xxHash-0.8.3/build && cmake ../cmake_unofficial -DBUILD_SHARED_LIBS=OFF -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_C_COMPILER=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang -DCMAKE_BUILD_TYPE=RelWithDebInfo
	cd temp/xxHash-0.8.3/build && make -j $(shell nproc)
	cd temp/xxHash-0.8.3/build && make install DESTDIR=$(shell pwd)/build
	/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/toolchains/hnpcli pack -i build/usr/local -n xxhash -v 0.8.3

download/v0.8.3.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/Cyan4973/xxHash/archive/refs/tags/v0.8.3.tar.gz