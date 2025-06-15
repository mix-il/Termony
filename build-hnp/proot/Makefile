all: download/proot
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && cp -vr ../download/proot proot
	mkdir -p temp/proot/build
	cd temp/proot/build && \
	make -f ../src/GNUmakefile loader.elf loader-m32.elf build.h && \
	make -f ../src/GNUmakefile proot && \
	make -f ../src/GNUmakefile test && \
	cp temp/proot/src/loader ./build/bin/ && \
	mkdir -p ../sysroot && \
	cp -rv ./build/. ../sysroot | tee file.lst

download/proot:
	mkdir -p download
	cd download && \
	rm -rf proot && git clone https://github.com/proot-me/proot.git && \
	git -C proot checkout bd5a5f6 # checkout v5.4.0
