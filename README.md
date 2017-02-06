# epOS, an eduational Intel 8086 32bit Operating system by Ramon Fried (2013-2016)

[![Build Status](https://travis-ci.org/mellowcandle/epOS.svg?branch=master)](https://travis-ci.org/mellowcandle/epOS) <a href="https://scan.coverity.com/projects/mellowcandle-epos">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/10219/badge.svg"/>
</a>

## Highlights

* Multiboot 1.0 compliancy
* Bitmap Page frame allocator
* Higer half kernel
* Paging enabled
* ACPICA
* APIC/IOAPIC
* APIC timer
* Complete vsprintf implementation (no floats :))

## In Progress
* PS/2 Keyboard driver (8042)
* Scheduler
* Ring 3 task execution

## Todo's

* Add testing framework


## Prerequisites

* Compilation:
$ sudo pacman -S grub mtools nasm libisoburn

* Qemu:
$ sudo pacman -S qemu qemu-arch-extra
