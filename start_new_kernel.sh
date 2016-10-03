#!/bin/bash

DEBUG=0
QEMU=0

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
	*)
            # unknown option
    ;;
esac
shift # past argument or value
done

if [[ $DEBUG -eq 1 ]]; then
	qemu-system-i386 -m 1G,slots=3,maxmem=4G -s -S -cdrom kernel.iso
else
	qemu-system-i386 -m 1G,slots=3,maxmem=4G -cdrom kernel.iso
fi

#qemu-system-i386 -kernel ./kernel.bin
