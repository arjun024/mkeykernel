#!/bin/bash
cd src/kernel

nasm -f elf32 kernel.asm -o kasm.o
nasm -f elf32 threadS.asm -o threadS.o
nasm -f elf32 interruptStubs.asm -o interruptStubs.o
gcc -m32 -c kernel.c -o kc.o
gcc -m32 -c io.c -o io.o
gcc -m32 -c lib.c -o lib.o
gcc -m32 -c gdt.c -o gdt.o
gcc -m32 -c physmem.c -o physmem.o
gcc -m32 -c thread.c -o thread.o
gcc -m32 -c interrupts.c -o interrupts.o
gcc -m32 -c idt.c -o idt.o
gcc -m32 -c pit.c -o pit.o
gcc -m32 -c pic.c -o pic.o

ld -m elf_i386 -T link.ld -o kernel kasm.o kc.o io.o lib.o gdt.o physmem.o thread.o threadS.o interrupts.o interruptStubs.o idt.o pit.o pic.o

echo "kernel build end, making .iso"

# make iso
cd ../../
mkdir -p iso/boot/grub              # create the folder structure
cp stage2_eltorito iso/boot/grub/   # copy the bootloader
cp menu.lst iso/boot/grub/          # copy menu
cp src/kernel/kernel iso/boot/      # copy the kernel

genisoimage -R                  \
-b boot/grub/stage2_eltorito    \
-no-emul-boot                   \
-boot-load-size 4               \
-A os                           \
-input-charset utf8             \
-quiet                          \
-boot-info-table                \
-o os.iso                       \
iso

