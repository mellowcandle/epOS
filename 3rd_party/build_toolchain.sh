#!/bin/bash
set -e

CWD="$(pwd)"
CORES=$(cat /proc/cpuinfo | awk '/^processor/{print $3}' | tail -1)

wget ftp.gnu.org/gnu/binutils/binutils-2.28.1.tar.xz
wget ftp://ftp.gnu.org/gnu/gcc/gcc-7.2.0/gcc-7.2.0.tar.gz

tar xf binutils-2.28.1.tar.xz
cd binutils-2.28.1
patch -s -p1 < ../binutils-2.28.1.diff

cd ..
tar xf gcc-7.2.0.tar.gz
cd gcc-7.2.0
patch -s -p1 < ../binutils-2.28.1.diff
cd ..


mkdir build_binutils
mkdir build_gcc
mkdir output

cd build_binutils
../binutils-2.28.1/configure --target=i686-epos --prefix=${CWD}../output --with-sysroot=${CWD}/../../overlay --disable-werror
make -j${CORES}
make install
cd ..

cd build_binutils
../gcc-7.2.0/configure --target=i686-epos --prefix=${CWD}../output --with-sysroot=${CWD}/../../overlay --enable-languages=c,c++
make all-gcc all-target-libgcc -j${CORES}
make all-target-libstdc++-v3 -j${CORES}
make install-gcc install-target-libgcc install-target-libstdc++-v3


