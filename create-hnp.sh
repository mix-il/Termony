#!/bin/sh
set -e -x
make -C build-hnp

wget -c https://github.com/Cyan4973/xxHash/archive/refs/tags/v0.8.3.tar.gz
rm -rf xxhash-source
mkdir xxhash-source
pushd xxhash-source
tar xvf ../v0.8.3.tar.gz
cd xxHash-0.8.3
mkdir build
cd build
cmake ../cmake_unofficial -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_C_COMPILER=/Applications/DevEco-Studio.app/Contents//sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=/
make -j $(nproc)
make install DESTDIR=$PWD/../../base
popd

wget -c https://github.com/lz4/lz4/releases/download/v1.10.0/lz4-1.10.0.tar.gz
rm -rf lz4-source
mkdir lz4-source
pushd lz4-source
tar xvf ../lz4-1.10.0.tar.gz
cd lz4-1.10.0
mkdir build-lz4
cd build-lz4
cmake ../build/cmake/ -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_C_COMPILER=/Applications/DevEco-Studio.app/Contents//sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=/
make -j $(nproc)
make install DESTDIR=$PWD/../../base
popd

wget -c https://github.com/Old-Man-Programmer/tree/archive/refs/tags/2.2.1.tar.gz
rm -rf tree-source
mkdir tree-source
pushd tree-source
tar xvf ../2.2.1.tar.gz
cd tree-2.2.1
make CC=/Applications/DevEco-Studio.app/Contents//sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang CFLAGS="-O3 -static -std=c11 -pedantic -Wall -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DLINUX" LDFLAGS="-static" PREFIX=/
make install DESTDIR=$PWD/../../base/bin PREFIX="/" MANDIR="$PWD/../../base/usr/share/man"
popd

wget -c https://mirrors.tuna.tsinghua.edu.cn/gnu/tar/tar-1.35.tar.xz
rm -rf tar-source
mkdir tar-source
pushd tar-source
tar xvf ../tar-1.35.tar.xz
cd tar-1.35
./configure --host aarch64-unknown-linux-musl --prefix=/ CC=/Applications/DevEco-Studio.app/Contents//sdk/default/openharmony/native/llvm/bin/aarch64-unknown-linux-ohos-clang AR=/Applications/DevEco-Studio.app/Contents//sdk/default/openharmony/native/llvm/bin/llvm-ar RANLIB=/Applications/DevEco-Studio.app/Contents//sdk/default/openharmony/native/llvm/bin/llvm-ranlib STRIP=/Applications/DevEco-Studio.app/Contents//sdk/default/openharmony/native/llvm/bin/llvm-strip
make -j $(nproc)
make install DESTDIR=$PWD/../../base
popd


/Applications/DevEco-Studio.app/Contents/sdk/default/openharmony/toolchains/hnpcli pack -i base -n base -v 0.0.1

rm -f ../entry/hnp/arm64-v8a/*.hnp
cp *.hnp ../entry/hnp/arm64-v8a/
