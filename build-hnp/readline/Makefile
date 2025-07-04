include ../utils/Makefrag

SOURCE_URL = https://ftpmirror.gnu.org/gnu/readline/readline-8.2.tar.gz
SOURCE_FILE = readline-8.2.tar.gz
SOURCE_DIR = readline-8.2
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --host aarch64-unknown-linux-musl

$(eval $(call define_autotools_package))
