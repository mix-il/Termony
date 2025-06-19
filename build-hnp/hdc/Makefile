include ../utils/Makefrag

all: download/hdc.zip download/securec.zip download/libusb.zip CMakeLists.txt
	rm -rf temp build
	mkdir -p temp build
	cd temp && unzip ../download/hdc.zip && mv developtools_hdc-OpenHarmony-v5.0.1-Release hdc
	cd temp && unzip ../download/securec.zip && mv third_party_bounds_checking_function-OpenHarmony-5.0.1-Release securec
	cp CMakeLists.txt temp
	#cd temp/hdc && git init . && git add --all && git commit -m "init" # help to update 0001-ignore-usb-fail.diff with git diff
	cd temp/hdc && cat ../../0001-ignore-usb-fail.diff | patch -Np1
	cd build && cmake ../temp -DCMAKE_TOOLCHAIN_FILE=${TOOL_HOME}/sdk/default/openharmony/native/build/cmake/ohos.toolchain.cmake -DOHOS_ARCH=arm64-v8a
	cd build && make -j$(nproc)
	mkdir -p ../sysroot/bin
	cp build/hdc ../sysroot/bin/

download/hdc.zip:
	mkdir -p download
	cd download && wget -O hdc.zip https://gitee.com/openharmony/developtools_hdc/repository/archive/OpenHarmony-v5.0.1-Release.zip

download/securec.zip:
	mkdir -p download
	cd download && wget -O securec.zip https://gitee.com/openharmony/third_party_bounds_checking_function/repository/archive/OpenHarmony-5.0.1-Release.zip
