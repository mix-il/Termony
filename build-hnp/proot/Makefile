all: download/proot
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && cp -r ../download/proot proot
	mkdir -p temp/proot/build
	cd temp/proot/build && \
	make -f ../src/GNUmakefile proot \
        PKG_CONFIG_PATH=../../../../sysroot/lib/pkgconfig \
        CFLAGS="--sysroot=$(TOOL_HOME)/sdk/default/openharmony/native/sysroot -O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer -D_FORTIFY_SOURCE=2" \
        LDFLAGS="--sysroot=$(TOOL_HOME)/sdk/default/openharmony/native/sysroot" \
        PKG_CONFIG_PATH=../../../../sysroot/lib/pkgconfig && \
    make -f ../src/GNUmakefile test
        CFLAGS="--sysroot=$(TOOL_HOME)/sdk/default/openharmony/native/sysroot -O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer -D_FORTIFY_SOURCE=2" \
        LDFLAGS="--sysroot=$(TOOL_HOME)/sdk/default/openharmony/native/sysroot" \
        PKG_CONFIG_PATH=../../../../sysroot/lib/pkgconfig && \
	cp temp/proot/src/loader ./build/bin/ && \
	mkdir -p ../sysroot && \
	cp -rv ./build/. ../sysroot | tee file.lst

download/proot:
	mkdir -p download
	cd download && \
	rm -rf proot && git clone https://github.com/termux/proot.git && \
