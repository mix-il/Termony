include ../utils/Makefrag

# required by mkbuiltin cross-build process when using gcc 13+
export CFLAGS_FOR_BUILD := -std=gnu17

SOURCE_URL = https://ftpmirror.gnu.org/gnu/bash/bash-5.2.37.tar.gz
SOURCE_FILE = bash-5.2.37.tar.gz
SOURCE_DIR = bash-5.2.37
CONFIG_ARGS = --prefix=$(PREFIX) --host aarch64-unknown-linux-musl --without-bash-malloc

$(eval $(call define_autotools_package))
