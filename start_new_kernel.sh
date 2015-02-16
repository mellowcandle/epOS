#!/bin/bash

# Copy the newly created kernel to place
cp kernel.bin /mnt/myfloppy
bochs -q
