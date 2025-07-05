include ../utils/Makefrag

all: download/zlib-1.3.1.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/zlib-1.3.1.tar.gz
	cd temp/zlib-1.3.1 && uname=Linux ./configure --prefix=/ --enable-shared
	cd temp/zlib-1.3.1 && make -j$(shell nproc)
	mkdir -p ../sysroot
	cd temp/zlib-1.3.1 && make install DESTDIR=$(shell pwd)/build
	cp -rfv ./build/. ../sysroot | tee file.lst

download/zlib-1.3.1.tar.gz:
	mkdir -p download
	cd download && rm -rf glib && wget https://zlib.net/zlib-1.3.1.tar.gz
