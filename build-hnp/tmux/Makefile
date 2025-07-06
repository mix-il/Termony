include ../utils/Makefrag

SOURCE_URL = https://github.com/tmux/tmux/releases/download/3.5a/tmux-3.5a.tar.gz
SOURCE_FILE = tmux-3.5a.tar.gz
SOURCE_DIR = tmux-3.5a
# TCSANOW does not work, use TCSADRAIN
PATCH_SOURCE = find temp/$(SOURCE_DIR) -type f | grep \\.c | xargs sed -i.bak 's/TCSANOW,/TCSADRAIN,/'
CONFIG_ARGS = --prefix=$(PREFIX) --host aarch64-unknown-linux-musl CFLAGS="-I $(shell pwd)/../sysroot/include -I $(shell pwd)/../sysroot/include/ncursesw"

$(eval $(call define_autotools_package))
