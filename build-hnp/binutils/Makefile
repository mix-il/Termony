include ../utils/Makefrag

SOURCE_URL = https://ftpmirror.gnu.org/gnu/binutils/binutils-2.44.tar.xz
SOURCE_FILE = binutils-2.44.tar.xz
SOURCE_DIR = binutils-2.44
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --host aarch64-unknown-linux-musl

$(eval $(call define_autotools_package))
