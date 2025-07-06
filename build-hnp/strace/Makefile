include ../utils/Makefrag

SOURCE_URL = https://github.com/strace/strace/releases/download/v6.15/strace-6.15.tar.xz
SOURCE_FILE = strace-6.15.tar.xz
SOURCE_DIR = strace-6.15
CONFIG_ARGS = --enable-mpers=no --prefix=$(PREFIX) --host aarch64-unknown-linux-musl CFLAGS="-Wno-error"

$(eval $(call define_autotools_package))
