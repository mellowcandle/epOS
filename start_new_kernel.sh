#!/bin/bash

DEBUG=0
QEMU=0
GFX="-nographic"
TRACES="-d guest_errors"
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -d|--debug)
    DEBUG=1
    shift # past argument
    ;;
	-b|--bochs)
	bochs -q
	exit $?
	;;
	-s|--simics)
	simics/simics simics/targets/x58-ich10/x58-ich10-dvdboot.simics
	exit $?
	;;
	-g|--graphics)
	GFX=
	shift
	;;
	*)
            # unknown option
    ;;
esac
shift # past argument or value
done

OPTIONS="${GFX} -m 1G,slots=3,maxmem=4G -cdrom kernel.iso"

if [[ $DEBUG -eq 1 ]]; then
	qemu-system-i386 ${OPTIONS} -s -S 2>&1 | tee run.log
else
	qemu-system-i386 ${OPTIONS} ${TRACES} 2>&1 | tee run.log
fi
