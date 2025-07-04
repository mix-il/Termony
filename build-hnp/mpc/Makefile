include ../utils/Makefrag

SOURCE_URL = https://ftpmirror.gnu.org/gnu/mpc/mpc-1.3.1.tar.gz
SOURCE_FILE = mpc-1.3.1.tar.gz
SOURCE_DIR = mpc-1.3.1
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --host aarch64-unknown-linux-musl

$(eval $(call define_autotools_package))
