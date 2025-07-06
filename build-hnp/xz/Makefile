include ../utils/Makefrag

export uname := Linux

SOURCE_URL = https://github.com/tukaani-project/xz/releases/download/v5.8.1/xz-5.8.1.tar.xz
SOURCE_FILE = xz-5.8.1.tar.xz
SOURCE_DIR = xz-5.8.1
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --host aarch64-unknown-linux-musl

$(eval $(call define_autotools_package))
