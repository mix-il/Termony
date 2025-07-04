include ../utils/Makefrag

SOURCE_URL = https://ftpmirror.gnu.org/gnu/mpfr/mpfr-4.2.2.tar.xz
SOURCE_FILE = mpfr-4.2.2.tar.xz
SOURCE_DIR = mpfr-4.2.2
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --host aarch64-unknown-linux-musl

$(eval $(call define_autotools_package))
