#!/bin/bash

# Copy the newly created kernel to place
cp kernel.bin /mnt/kernel/mykern
# Sync to make sure the kernel is written to loop device
sync

bochs -q
