include ../utils/Makefrag

SOURCE_URL = $(GNU_MIRROR)/gnu/gdb/gdb-16.3.tar.xz
SOURCE_FILE = gdb-16.3.tar.xz
SOURCE_DIR = gdb-16.3
CONFIG_ARGS = --prefix=$(PREFIX) --with-lzma --disable-sim --disable-static --enable-shared --with-system-readline --host $(OHOS_ARCH)-unknown-linux-musl --disable-nls

$(eval $(call define_autotools_package))
