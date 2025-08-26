include ../utils/Makefrag

SOURCE_URL = https://downloads.xiph.org/releases/opus/opus-1.5.2.tar.gz
SOURCE_FILE = opus-1.5.2.tar.gz
SOURCE_DIR = opus-1.5.2

CMAKE_ARGS = -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=$(OHOS_ARCH) -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo

$(eval $(call define_cmake_package))
