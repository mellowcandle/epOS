#!/bin/bash 

DEBUG=0
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -d|--debug)
    DEBUG=1
    shift # past argument
    ;;
    *)
            # unknown option
    ;;
esac
shift # past argument or value
done

if [[ $DEBUG -eq 1 ]]; then
	qemu-system-i386 -s -S -cdrom kernel.iso
else
	qemu-system-i386 -cdrom kernel.iso
fi
#qemu-system-i386 -kernel ./kernel.bin
