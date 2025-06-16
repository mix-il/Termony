all: download/libarchive-3.8.1.tar.gz
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && tar xvf ../download/libarchive-3.8.1.tar.gz
	cd temp/libarchive-3.8.1
	cd temp && cmake -S libarchive-3.8.1 -B libarchive-3.8.1/build-ohos \
	-DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF \
	-DCMAKE_C_COMPILER=$(OHOS_SDK_HOME)/native/llvm/bin/aarch64-unknown-linux-ohos-clang  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DENABLE_TEST=OFF \
	-DENABLE_PCREPOSIX=OFF \
	-DENABLE_PCRE2POSIX=OFF \
	-DENABLE_LIBB2=OFF \
	-DENABLE_LZMA=OFF \
	-DENABLE_EXPAT=OFF \
	-DOPENSSL_USE_STATIC_LIBS=OFF \
	-DCMAKE_PREFIX_PATH=../../sysroot \
	-DCMAKE_INCLUDE_PATH=../../sysroot/include \
	-DOPENSSL_INCLUDE_DIR=../../sysroot/include \
	-DOPENSSL_SSL_LIBRARY=../../sysroot/lib/libssl.so \
	-DOPENSSL_CRYPTO_LIBRARY=../../sysroot/lib/libcrypto.so \
	-DLZ4_LIBRARY=../../sysroot/lib/liblz4.so \
	-DZSTD_LIBRARY=../../sysroot/lib/libzstd.so
	cd temp/libarchive-3.8.1/build-ohos && make -j $(shell nproc)
	cd temp/libarchive-3.8.1/build-ohos && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rv build/usr/local/. ../sysroot/ | tee file.lst

download/libarchive-3.8.1.tar.gz:
	mkdir -p download
	cd download && wget https://www.libarchive.org/downloads/libarchive-3.8.1.tar.gz
