include ../utils/Makefrag

SOURCE_URL = $(GNU_MIRROR)/gnu/readline/readline-8.3.tar.gz
SOURCE_FILE = readline-8.3.tar.gz
SOURCE_DIR = readline-8.3
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --host $(OHOS_ARCH)-unknown-linux-musl

$(eval $(call define_autotools_package))
