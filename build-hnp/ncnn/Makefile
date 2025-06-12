all: download/ncnn-20250503-full-source.zip
	rm -rf temp build
	mkdir -p temp/ncnn-20250503 build
	cd temp/ncnn-20250503 && unzip -o ../../download/ncnn-20250503-full-source.zip && mkdir build
	cd temp/ncnn-20250503/build && PKG_CONFIG=/usr/bin/false CFLAGS="-O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer -nodefaultlibs -fno-builtin -fno-stack-protector -nostdinc++ -mno-outline-atomics -lc"  CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" LDFLAGS="-L$(shell pwd)/../sysroot/lib" cmake ../ -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_INSTALL_PREFIX=/usr/local/ -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DNCNN_SHARED_LIB=ON -DNCNN_SIMPLEOCV=ON -DNCNN_BUILD_EXAMPLES=ON -DNCNN_STDIO=ON -DNCNN_STRING=ON -DNCNN_BUILD_BENCHMARK=ON -DNCNN_OPENMP=OFF -DNCNN_VULKAN=ON -DCMAKE_C_COMPILER=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang -DCMAKE_CXX_COMPILER=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang++ AR=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip  -DCMAKE_BUILD_TYPE=RelWithDebInfo 
	cd temp/ncnn-20250503/build && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p build/usr/local/share/benchncnn
	cp -rfv temp/ncnn-20250503/benchmark/*.param build/usr/local/share/benchncnn/
	cp -rfv temp/ncnn-20250503/build/benchmark/benchncnn build/usr/local/share/benchncnn/
	mkdir -p ../sysroot
	$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/llvm-strip build/usr/local/lib/*.so
	cp -rfv build/usr/local/. ../sysroot | tee file.lst

download/ncnn-20250503-full-source.zip:
	mkdir -p download
	cd download && wget https://github.com/Tencent/ncnn/releases/download/20250503/ncnn-20250503-full-source.zip