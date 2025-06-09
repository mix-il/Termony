all: download/busybox-1.37.0.tar.bz2
	rm -rf build
	mkdir -p build
	cd build && tar xvf ../download/busybox-1.37.0.tar.bz2
	cd build/busybox-1.37.0 && make defconfig
	# missing getutxent
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/CONFIG_W=y/CONFIG_W=n/" .config
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/CONFIG_WHO=y/CONFIG_WHO=n/" .config
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/CONFIG_USERS=y/CONFIG_USERS=n/" .config
	# fix compilation
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/CONFIG_SHA1_HWACCEL=y/CONFIG_SHA1_HWACCEL=n/" .config
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/CONFIG_HUSH=y/CONFIG_HUSH=n/" .config
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/CONFIG_ETHER_WAKE=y/CONFIG_ETHER_WAKE=n/" .config
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/CONFIG_FEATURE_UTMP=y/CONFIG_FEATURE_UTMP=n/" .config
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/CONFIG_SU=y/CONFIG_SU=n/" .config
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/CONFIG_LS=y/CONFIG_LS=n/" .config
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/CONFIG_SORT=y/CONFIG_SORT=n/" .config
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/CONFIG_HOSTID=y/CONFIG_HOSTID=n/" .config
	# avoid being killed by seccomp
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/if (setgid(gid))/\\/\\//" libbb/xfuncs_printf.c
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/setgid(rgid)/\\/\\//" libbb/appletlib.c
	cd build/busybox-1.37.0 && /usr/bin/sed -i.bak "s/setuid(ruid)/\\/\\//" libbb/appletlib.c
	cd build/busybox-1.37.0 && make -j $(shell proc) CC=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-ar CFLAGS="-Wno-int-conversion" busybox_unstripped
	/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/native/llvm/bin/llvm-strip build/busybox-1.37.0/busybox_unstripped
	mkdir -p ../sysroot/bin
	cp -v build/busybox-1.37.0/busybox_unstripped ../sysroot/bin/busybox

download/busybox-1.37.0.tar.bz2:
	mkdir -p download
	cd download && wget https://busybox.net/downloads/busybox-1.37.0.tar.bz2