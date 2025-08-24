include ../utils/Makefrag

SOURCE_URL = $(GNU_MIRROR)/gnu/binutils/binutils-2.44.tar.xz
SOURCE_FILE = binutils-2.44.tar.xz
SOURCE_DIR = binutils-2.44
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --host $(OHOS_ARCH)-unknown-linux-musl

$(eval $(call define_autotools_package))
