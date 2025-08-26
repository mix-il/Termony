include ../utils/Makefrag

SOURCE_URL = https://github.com/lz4/lz4/releases/download/v1.10.0/lz4-1.10.0.tar.gz
SOURCE_FILE = lz4-1.10.0.tar.gz
SOURCE_DIR = lz4-1.10.0
CMAKE_LISTS_PATH = ../build/cmake
CMAKE_ARGS = -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=$(OHOS_ARCH) -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo

$(eval $(call define_cmake_package))
