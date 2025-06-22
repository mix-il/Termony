include ../utils/Makefrag

all: download/qemu-10.0.2.tar.xz
	rm -rf temp build
	mkdir -p temp build/bin
	cd temp && tar xvf ../download/qemu-10.0.2.tar.xz
	cd temp/qemu-10.0.2 && cat ../../patch/linuxload.c.patch | patch -Np1
	cd temp/qemu-10.0.2 && cat ../../patch/signal.c.patch | patch -Np1
	cd temp/qemu-10.0.2 && cat ../../patch/strace.c.patch | patch -Np1
	cd temp/qemu-10.0.2 && cat ../../patch/syscall.c.patch | patch -Np1
	cd temp/qemu-10.0.2 && cat ../../patch/syscall64.tbl.patch | patch -Np1
	cd temp/qemu-10.0.2 && cat ../../patch/elfload.c.patch | patch -Np1
	cd temp/qemu-10.0.2 && \
	PKG_CONFIG=$(shell which pkg-config) \
	PKG_CONFIG_PATH= \
	PKG_CONFIG_LIBDIR=$(shell pwd)/../sysroot/lib/pkgconfig:$(shell pwd)/../sysroot/share/pkgconfig \
	PKG_CONFIG_SYSROOT_DIR=$(shell pwd)/../sysroot \
	CFLAGS="-D_UAPI_LINUX_VIRTIO_VSOCK_H -D_UAPI_LINUX_VIRTIO_TYPES_H -D_UAPI_LINUX_VIRTIO_RING_H -D_UAPI_LINUX_VIRTIO_PMEM_H -D_UAPI_LINUX_VIRTIO_NET_H -D_UAPI_LINUX_VIRTIO_IOMMU_H -D_UAPI_LINUX_VIRTIO_FS_H -D_UAPI_LINUX_VIRTIO_CONSOLE_H -D_UAPI_LINUX_VIRTIO_CONFIG_H -D_LINUX_SYSINFO_H -UHAVE_OPENAT2_H -UTARGET_NR_mq_open -D__user= -D__force= ${CFLAGS}" \
	 ./configure --target-list=aarch64-linux-user --cross-prefix= --host-cc=cc --disable-kvm --disable-xen --disable-rust --disable-docs --disable-system --enable-tcg-interpreter
	cd temp/qemu-10.0.2 && make -j $(shell nproc) && make install DESTDIR=$(shell pwd)/build
	mkdir -p ../sysroot
	cp -rv ./build/usr/local/. ../sysroot | tee file.lst

download/qemu-10.0.2.tar.xz:
	mkdir -p download
	cd download && wget https://download.qemu.org/qemu-10.0.2.tar.xz
