all: make.hnp

make.hnp: download/make-4.4.1.tar.gz
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && tar xvf ../download/make-4.4.1.tar.gz
	cd temp/make-4.4.1 && ./configure --host aarch64-unknown-linux-musl --prefix=/ CC=/Applications/DevEco-Studio.app/Contents//sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=/Applications/DevEco-Studio.app/Contents//sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=/Applications/DevEco-Studio.app/Contents//sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=/Applications/DevEco-Studio.app/Contents//sdk/default/openharmony/native/llvm/bin/llvm-strip
	cd temp/make-4.4.1 && make -j $(shell nproc)
	cd temp/make-4.4.1 && make install DESTDIR=$(shell pwd)/build
	/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/toolchains/hnpcli pack -i build -n make -v 4.4.1

download/make-4.4.1.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/make/make-4.4.1.tar.gz