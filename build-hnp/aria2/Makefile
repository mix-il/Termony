include ../utils/Makefrag

SOURCE_URL = https://github.com/aria2/aria2/releases/download/release-1.37.0/aria2-1.37.0.tar.gz
SOURCE_FILE = aria2-1.37.0.tar.gz
SOURCE_DIR = aria2-1.37.0
CONFIG_ARGS = --prefix=$(PREFIX) --host aarch64-unknown-linux-musl --disable-nls --with-openssl --without-gnutls OPENSSL_CFLAGS="-D_FORTIFY_SOURCE=2" OPENSSL_LIBS="-lssl -lcrypto" --with-sysroot=$(OHOS_SDK_HOME)/native/sysroot --enable-libaria2 --with-ca-bundle=/etc/ssl/certs/cacert.pem CFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2 -O2 -g -pipe -fstack-protector-strong -fno-omit-frame-pointer" CPPFLAGS="-I$(shell pwd)/../sysroot/include -D_FORTIFY_SOURCE=2" LDFLAGS="-L$(shell pwd)/../sysroot/lib"

$(eval $(call define_autotools_package))
