include ../utils/Makefrag

SOURCE_URL = https://github.com/libevent/libevent/releases/download/release-2.1.12-stable/libevent-2.1.12-stable.tar.gz
SOURCE_FILE = libevent-2.1.12-stable.tar.gz
SOURCE_DIR = libevent-2.1.12-stable
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --disable-openssl --host aarch64-unknown-linux-musl

$(eval $(call define_autotools_package))
