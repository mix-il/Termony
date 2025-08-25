include ../utils/Makefrag

SOURCE_URL = $(GNU_MIRROR)/gnu/gmp/gmp-6.3.0.tar.xz
SOURCE_FILE = gmp-6.3.0.tar.xz
SOURCE_DIR = gmp-6.3.0
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --host $(OHOS_ARCH)-unknown-linux-musl

$(eval $(call define_autotools_package))
