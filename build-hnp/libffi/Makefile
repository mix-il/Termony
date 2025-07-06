include ../utils/Makefrag

SOURCE_URL = https://github.com/libffi/libffi/releases/download/v3.4.5/libffi-3.4.5.tar.gz
SOURCE_FILE = libffi-3.4.5.tar.gz
SOURCE_DIR = libffi-3.4.5
CONFIG_ARGS = --prefix=$(PREFIX) --enable-static --enable-shared --host aarch64-unknown-linux-musl

$(eval $(call define_autotools_package))
