include ../utils/Makefrag

all: download/proot
	rm -rf temp build
	mkdir -p temp build/bin
	cp queue.h ../sysroot/include
	cd temp && cp -r ../download/proot proot
	cd temp/proot && git apply ../../ohos.patch
	mkdir -p temp/proot/build
	cd temp/proot/build && \
	PKG_CONFIG=/opt/homebrew/bin/pkg-config \
	PKG_CONFIG_PATH=../../../../sysroot/lib/pkgconfig \
	CC=$(CC) \
	CFLAGS="--sysroot=$(OHOS_SDK_HOME)/native/sysroot -O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer -D_FORTIFY_SOURCE=2 $(CFLAGS)" \
	LDFLAGS="--sysroot=$(OHOS_SDK_HOME)/native/sysroot -L$(OHOS_SDK_HOME)/native/sysroot/usr/lib $(LDFLAGS)" \
	OBJCOPY=$(OBJCOPY) \
	READELF=$(READELF) \
	PKG_CONFIG_PATH=../../../../sysroot/lib/pkgconfig \
	make -C ../src loader && \
	PKG_CONFIG=/opt/homebrew/bin/pkg-config \
	PKG_CONFIG_PATH=../../../../sysroot/lib/pkgconfig \
	CC=$(CC) \
	CFLAGS="--sysroot=$(OHOS_SDK_HOME)/native/sysroot -O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer -D_FORTIFY_SOURCE=2 -D__OHOS__ $(CFLAGS)" \
	CPPFLAGS="-D__OHOS__" \
	LDFLAGS="--sysroot=$(OHOS_SDK_HOME)/native/sysroot -L$(OHOS_SDK_HOME)/native/sysroot/usr/lib $(LDFLAGS)" \
	OBJCOPY=$(OBJCOPY) \
	READELF=$(READELF) \
	PKG_CONFIG_PATH=../../../../sysroot/lib/pkgconfig \
	make -C ../src proot
	cp temp/proot/src/proot ./build/bin/
	mkdir -p ../sysroot && \
	cp -rv ./build/. ../sysroot | tee file.lst

download/proot:
	mkdir -p download
	cd download && \
	rm -rf proot && git clone https://github.com/termux/proot.git --recursive && \
	cd proot && git checkout 60485d2646c1e09105099772da4a20deda8d020d
