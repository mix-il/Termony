all: download/6703f7504a38a8da96b353cadafa64d3c2d7a2d3.zip download/3b92a251022474a293cefc1291bc4fe7b78b62e7.zip
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && unzip -o ../download/6703f7504a38a8da96b353cadafa64d3c2d7a2d3.zip
	cd temp/STREAM-6703f7504a38a8da96b353cadafa64d3c2d7a2d3 && make stream_c.exe CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang CFLAGS="-O3 -static -DNDEBUG -DSTREAM_ARRAY_SIZE=120000000 -DNTIMES=200" LDFLAGS="-static"
	cd temp/STREAM-6703f7504a38a8da96b353cadafa64d3c2d7a2d3 && cp -vf stream_c.exe $(shell pwd)/build/bin

	cd temp && unzip -o ../download/3b92a251022474a293cefc1291bc4fe7b78b62e7.zip
	cd temp/stream-3b92a251022474a293cefc1291bc4fe7b78b62e7 && make V=1 KERNEL=zfill-acle all CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang CFLAGS="-march=armv8.5-a+sve -O3 -static -DNDEBUG -DSTREAM_ARRAY_SIZE=120000000 -DNTIMES=200"  LDFLAGS="-static -lomp"
	cd temp/stream-3b92a251022474a293cefc1291bc4fe7b78b62e7 && cp -vf stream_zfill-acle.exe $(shell pwd)/build/bin/

	cd temp/stream-3b92a251022474a293cefc1291bc4fe7b78b62e7 && make V=1 KERNEL=zfill all CC=$(TOOL_HOME)/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang CFLAGS="-O3 -static -DNDEBUG -DSTREAM_ARRAY_SIZE=120000000 -DNTIMES=200"  LDFLAGS="-static -lomp"
	cd temp/stream-3b92a251022474a293cefc1291bc4fe7b78b62e7 && cp -vf stream_zfill.exe $(shell pwd)/build/bin/

	mkdir -p ../sysroot
	cp -rv ./build/. ../sysroot | tee file.lst

download/6703f7504a38a8da96b353cadafa64d3c2d7a2d3.zip:
	mkdir -p download
	cd download && wget https://github.com/jeffhammond/STREAM/archive/6703f7504a38a8da96b353cadafa64d3c2d7a2d3.zip

download/3b92a251022474a293cefc1291bc4fe7b78b62e7.zip:
	mkdir -p download
	cd download && wget https://github.com/jlinford/stream/archive/3b92a251022474a293cefc1291bc4fe7b78b62e7.zip