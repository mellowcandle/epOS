#!/bin/bash

set -e

CORES=$(cat /proc/cpuinfo | awk '/^processor/{print $3}' | tail -1)
# The cool line below will set ROOT DIR to this path, will allow running this script from outside the makefile
: ${ROOTDIR:="$(pwd)/.."}

TOOLCHAIN_PATH="toolchain/i686-elf-7.2.0-epos-x86_64/bin/"
PATH=$PATH:${ROOTDIR}/${TOOLCHAIN_PATH}
SYSROOT="${ROOTDIR}/overlay"

if [ "$#" -ge 1 ]; then
	if [ $1 == "clean" ]
		then
			rm -rf ${ROOTDIR}/build
	exit 0
	fi
fi

mkdir -p ${ROOTDIR}/build
cd ${ROOTDIR}/build
${ROOTDIR}/3rd_party/newlib-2.5.0.20170818/configure --prefix=/usr --target=i686-epos
make -j${CORES}
make DESTDIR=${SYSROOT} install
cp -ar $SYSROOT/usr/i686-epos/* $SYSROOT/usr/
