#!/bin/bash

set -e

CWD="$(pwd)"


# Clean everything in toolchain
rm -rf toolchain/bin toolchain/build
rm -rf toolchain/i686-elf-7.2.0-epos-x86_64
rm -rf toolchain/autoconf-2.65
rm -rf toolchain/automake-1.12

# extract necessary files
tar xvf toolchain/i686-elf-7.2.0-epos-x86_64.tar.bz2 -C toolchain
tar xvf toolchain/autoconf-2.65.tar.gz -C toolchain
tar xvf toolchain/automake-1.12.tar.gz -C toolchain

mkdir -p toolchain/bin toolchain/build

# Build automak and autoconf
pushd .
cd toolchain/build;
${CWD}/toolchain/automake-1.12/configure --prefix="${CWD}/toolchain"
make && make install
${CWD}/toolchain/autoconf-2.65/configure --prefix="${CWD}/toolchain"
make && make install
popd

ln -sf ${CWD}/kernel/include/uapi/syscall.h ${CWD}/3rd_party/newlib-2.5.0.20170818/newlib/libc/sys/epos/syscall.h
