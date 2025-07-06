include ../utils/Makefrag

SOURCE_URL = https://github.com/Cyan4973/xxHash/archive/refs/tags/v0.8.3.tar.gz
SOURCE_FILE = v0.8.3.tar.gz
SOURCE_DIR = xxHash-0.8.3
CMAKE_LISTS_PATH = ../cmake_unofficial
CMAKE_ARGS = -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo

$(eval $(call define_cmake_package))
