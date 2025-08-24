include ../utils/Makefrag

SOURCE_URL = https://github.com/htop-dev/htop/releases/download/3.4.1/htop-3.4.1.tar.xz
SOURCE_FILE = htop-3.4.1.tar.xz
SOURCE_DIR = htop-3.4.1
PATCH_SOURCE = cd temp/$(SOURCE_DIR) && cat ../../0001-skip-proc-stat.diff | patch -Np1

CONFIG_ARGS = --prefix=$(PREFIX) --host $(OHOS_ARCH)-unknown-linux-musl --enable-affinity CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2"

$(eval $(call define_autotools_package))
