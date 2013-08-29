#!/bin/bash

# First let's unmount the floppy if it's already mounted
sudo umount /mnt
sudo losetup -d /dev/loop1

# Now, let's mount it
sudo losetup /dev/loop1 floppy.img
sudo mount -o loop /dev/loop1 /mnt

# Copy the newly created kernel to place
cp kernel.bin /mnt/mykern
bochs -q
