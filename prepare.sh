#!/bin/bash
CWD="$(pwd)"

# Clean everything in toolchain
rm -rf toolchain/bin toolchain/build
rm -rf toolchain/i686-elf-4.9.1-Linux-x86_64
rm -rf toolchain/autoconf-2.65
rm -rf toolchain/automake-1.12

# extract necessary files
tar xvf toolchain/i686-elf-4.9.1-Linux-x86_64.tar.xz -C toolchain
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

# Create symlinks for easier digestion
ln -s ${CWD}/toolchain/i686-elf-4.9.1-Linux-x86_64/bin/i686-elf-ar ${CWD}/toolchain/bin/i686-epos-ar
ln -s ${CWD}/toolchain/i686-elf-4.9.1-Linux-x86_64/bin/i686-elf-as ${CWD}/toolchain/bin/i686-epos-as
ln -s ${CWD}/toolchain/i686-elf-4.9.1-Linux-x86_64/bin/i686-elf-gcc ${CWD}/toolchain/bin/i686-epos-gcc
ln -s ${CWD}/toolchain/i686-elf-4.9.1-Linux-x86_64/bin/i686-elf-gcc ${CWD}/toolchain/bin/i686-epos-cc
ln -s ${CWD}/toolchain/i686-elf-4.9.1-Linux-x86_64/bin/i686-elf-ranlib ${CWD}/toolchain/bin/i686-epos-ranlib

