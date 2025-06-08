all: download/busybox-static-1.37.0-r18.apk
	rm -rf build
	mkdir -p build
	cd build && tar xvf ../download/busybox-static-1.37.0-r18.apk
	mkdir -p ../sysroot/bin
	cp -v build/bin/busybox.static ../sysroot/bin

download/busybox-static-1.37.0-r18.apk:
	mkdir -p download
	cd download && wget https://mirrors.tuna.tsinghua.edu.cn/alpine/latest-stable/main/aarch64/busybox-static-1.37.0-r18.apk