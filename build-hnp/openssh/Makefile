
all: download/V_10_0_P2.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/V_10_0_P2.tar.gz
	cd temp/openssh-portable-V_10_0_P2 && cat ../../0001-fix-getpwuid.diff | patch -Np1
	cd temp/openssh-portable-V_10_0_P2 && PKG_CONFIG=/usr/bin/false ./configure --without-shadow --disable-etc-default-login --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --without-ssh1 ac_cv_func_readpassphrase=no ac_cv_lib_crypt_crypt=no --host aarch64-unknown-linux-musl CC=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-strip CFLAGS="-O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer"  CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" LDFLAGS="-L$(shell pwd)/../sysroot/lib"
	cd temp/openssh-portable-V_10_0_P2 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot

download/V_10_0_P2.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/openssh/openssh-portable/archive/refs/tags/V_10_0_P2.tar.gz