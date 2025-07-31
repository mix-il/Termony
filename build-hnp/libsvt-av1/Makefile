include ../utils/Makefrag

SOURCE_URL = https://gitlab.com/AOMediaCodec/SVT-AV1/-/archive/v1.8.0/SVT-AV1-v1.8.0.tar.bz2
SOURCE_FILE = SVT-AV1-v1.8.0.tar.bz2
SOURCE_DIR = SVT-AV1-v1.8.0

# fix missing pthread_setaffinity_np
PATCH_SOURCE = cd temp/$(SOURCE_DIR) && sed -i '/pthread_setaffinity_np/d' Source/Lib/Common/Codec/EbThreads.h

CMAKE_ARGS = -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo

$(eval $(call define_cmake_package))
