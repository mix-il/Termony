all:  download/main.zip
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && unzip ../download/main.zip
	mkdir -p ../sysroot/bin
	cd temp/coremark-main && make clean && CC=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang make XCFLAGS="-O3 -static" compile && cp -v coremark.exe $(shell pwd)/../sysroot/bin/coremark.exe
	cd temp/coremark-main && make clean && CC=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang make XCFLAGS="-O3 -DMULTITHREAD=20 -DUSE_PTHREAD -pthread -static" compile && cp -v coremark.exe $(shell pwd)/../sysroot/bin/coremark_p20c.exe

download/main.zip:
	mkdir -p download
	cd download && wget https://github.com/eembc/coremark/archive/refs/heads/main.zip