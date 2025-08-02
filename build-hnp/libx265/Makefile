# does not work
export USE_CCACHE := 0

include ../utils/Makefrag

SOURCE_URL = http://ftp.videolan.org/pub/videolan/x265/x265_4.1.tar.gz
SOURCE_FILE = x265_4.1.tar.gz
SOURCE_DIR = x265_4.1/source
PATCH_SOURCE = cd temp/$(SOURCE_DIR)/.. && cat ../../0001-cmake-fix.patch | patch -Np1
CMAKE_ARGS = -DCMAKE_INSTALL_PREFIX=$(PREFIX) -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_POLICY_VERSION_MINIMUM=3.5

$(eval $(call define_cmake_package))
