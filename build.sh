#!/usr/bin/env bash
set -euo pipefail
rm -rf build iso os.iso
mkdir -p build iso/boot/grub
cc -m32 -ffreestanding -fno-pic -fno-pie -O2 -Wall -Wextra -c kernel/boot.S -o build/boot.o
cc -m32 -ffreestanding -fno-pic -fno-pie -O2 -Wall -Wextra -c kernel/kernel.c -o build/kernel.o
ld -m elf_i386 -T kernel/linker.ld -nostdlib -o build/kernel.elf build/boot.o build/kernel.o
cp build/kernel.elf iso/boot/kernel.elf
cat > iso/boot/grub/grub.cfg <<'GRUB'
set timeout=0
set default=0
menuentry "Agentic OS" {
    multiboot /boot/kernel.elf
    boot
}
GRUB
grub-mkrescue -o os.iso iso >/dev/null 2>&1
echo "Built os.iso"
