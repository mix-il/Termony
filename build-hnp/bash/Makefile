include ../utils/Makefrag

# required by mkbuiltin cross-build process when using gcc 13+
export CFLAGS_FOR_BUILD := -std=gnu17

SOURCE_URL = $(GNU_MIRROR)/gnu/bash/bash-5.3.tar.gz
SOURCE_FILE = bash-5.3.tar.gz
SOURCE_DIR = bash-5.3
CONFIG_ARGS = --prefix=$(PREFIX) --host $(OHOS_ARCH)-unknown-linux-musl --without-bash-malloc --disable-nls

$(eval $(call define_autotools_package))
