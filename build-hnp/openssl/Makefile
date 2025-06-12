all: download/openssl-3.5.0.tar.gz
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && tar xvf ../download/openssl-3.5.0.tar.gz
	cd temp/openssl-3.5.0 && cat ../../0001-ohos-getauxval.diff | patch -Np1
	cd temp/openssl-3.5.0 && ./Configure linux-aarch64 shared no-tests -D__OHOS__ CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip
	cd temp/openssl-3.5.0 && make -j $(shell nproc)
	cd temp/openssl-3.5.0 && make install_sw DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	rm -f ./build/usr/local/lib/*.a
	cp -rv ./build/usr/local/. ../sysroot | tee file.lst

download/openssl-3.5.0.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/openssl/openssl/releases/download/openssl-3.5.0/openssl-3.5.0.tar.gz
