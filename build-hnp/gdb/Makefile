include ../utils/Makefrag

SOURCE_URL = https://ftpmirror.gnu.org/gnu/gdb/gdb-16.3.tar.xz
SOURCE_FILE = gdb-16.3.tar.xz
SOURCE_DIR = gdb-16.3
CONFIG_ARGS = --prefix=$(PREFIX) --with-lzma --disable-sim --disable-static --enable-shared --with-system-readline --host aarch64-unknown-linux-musl

$(eval $(call define_autotools_package))
