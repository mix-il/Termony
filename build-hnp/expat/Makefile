include ../utils/Makefrag

SOURCE_URL = https://github.com/libexpat/libexpat/releases/download/R_2_7_1/expat-2.7.1.tar.xz
SOURCE_FILE = expat-2.7.1.tar.xz
SOURCE_DIR = expat-2.7.1
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --host $(OHOS_ARCH)-unknown-linux-musl

$(eval $(call define_autotools_package))
