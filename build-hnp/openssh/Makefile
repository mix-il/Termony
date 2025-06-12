
all: download/V_10_0_P2.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/V_10_0_P2.tar.gz
	cd temp/openssh-portable-V_10_0_P2 && cat ../../0001-fix-getpwuid.diff | patch -Np1
	cd temp/openssh-portable-V_10_0_P2 && PKG_CONFIG=/usr/bin/false ./configure --prefix=/data/app/base.org/base_1.0 --without-shadow --disable-etc-default-login --disable-utmp --disable-utmpx --disable-wtmp --disable-wtmpx --without-ssh1 ac_cv_func_readpassphrase=no ac_cv_lib_crypt_crypt=no --host aarch64-unknown-linux-musl CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib CFLAGS="-O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer"  CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" LDFLAGS="-L$(shell pwd)/../sysroot/lib"
	cd temp/openssh-portable-V_10_0_P2 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build STRIP_OPT="-s --strip-program=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip"
	mkdir -p ../sysroot
	cp -rfv build/data/app/base.org/base_1.0/. ../sysroot | tee file.lst

download/V_10_0_P2.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/openssh/openssh-portable/archive/refs/tags/V_10_0_P2.tar.gz
