include ../utils/Makefrag

SOURCE_URL = https://github.com/libusb/libusb/releases/download/v1.0.29/libusb-1.0.29.tar.bz2
SOURCE_FILE = libusb-1.0.29.tar.bz2
SOURCE_DIR = libusb-1.0.29
CONFIG_ARGS = --prefix=$(PREFIX) --disable-static --enable-shared --disable-udev --host $(OHOS_ARCH)-unknown-linux-musl
AFTER_INSTALL = ln -sv libusb-1.0 build$(PREFIX)/include/libusb # hdc need this

$(eval $(call define_autotools_package))
