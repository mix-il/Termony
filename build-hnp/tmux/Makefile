all: download/tmux-3.5a.tar.gz
	rm -rf temp build
	mkdir -p temp build
	cd temp && tar xvf ../download/tmux-3.5a.tar.gz
	# TCSANOW does not work, use TCSADRAIN
	find temp/tmux-3.5a -type f | grep \\.c | xargs sed -i.bak 's/TCSANOW,/TCSADRAIN,/'
	cd temp/tmux-3.5a && mkdir build
	cd temp/tmux-3.5a/build && PKG_CONFIG=/usr/bin/true ../configure --host aarch64-unknown-linux-musl CC=$(OHOS_SDK_HOME)/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ar RANLIB=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-ranlib STRIP=$(OHOS_SDK_HOME)/native/llvm/bin/llvm-strip CFLAGS="-I $(shell pwd)/../sysroot/include -I $(shell pwd)/../sysroot/include/ncursesw" LDFLAGS="-L $(shell pwd)/../sysroot/lib -lncursesw -ltinfow -levent"
	cd temp/tmux-3.5a/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/tmux-3.5a.tar.gz:
	mkdir -p download
	cd download && wget https://github.com/tmux/tmux/releases/download/3.5a/tmux-3.5a.tar.gz
