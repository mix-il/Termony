include ../utils/Makefrag

SOURCE_URL = https://github.com/mstorsjo/fdk-aac/archive/refs/tags/v2.0.3.tar.gz
SOURCE_FILE = v2.0.3.tar.gz
SOURCE_DIR = fdk-aac-2.0.3
CMAKE_ARGS = -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo

$(eval $(call define_cmake_package))
