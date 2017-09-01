#!/bin/bash

set -e

CORES=$(cat /proc/cpuinfo | awk '/^processor/{print $3}' | tail -1)
# The cool line below will set ROOT DIR to this path, will allow running this script from outside the makefile

PATH=$PATH:${TOOLCHAIN_PATH}
SYSROOT="${EPOS_ROOTDIR}/overlay"

if [ "$#" -ge 1 ]; then
	if [ $1 == "clean" ]
		then
			rm -rf ${EPOS_ROOTDIR}/build
	exit 0
	fi
fi

mkdir -p ${EPOS_ROOTDIR}/build
mkdir -p ${EPOS_SYSROOT}
cd ${EPOS_ROOTDIR}/build
${EPOS_ROOTDIR}/3rd_party/newlib-2.5.0.20170818/configure --prefix=/usr --target=i686-epos
make -j${CORES}
make DESTDIR=${EPOS_SYSROOT} install
cp -ar $EPOS_SYSROOT/usr/i686-epos/* $EPOS_SYSROOT/usr/
