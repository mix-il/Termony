all: bash.hnp

bash.hnp: download/bash-5.2.37.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/bash-5.2.37.tar.gz
	cd temp/bash-5.2.37 && mkdir build
	cd temp/bash-5.2.37/build && ../configure --host aarch64-unknown-linux-musl --disable-readline --without-bash-malloc CC=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang --prefix=/
	cd temp/bash-5.2.37/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/toolchains/hnpcli pack -i build -n bash -v 5.2.37

download/bash-5.2.37.tar.gz:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/gnu/bash/bash-5.2.37.tar.gz