# Agentic OS State

Current date: 2026-07-20

## Current milestone
- M2 core CPU infrastructure completed for the current 32-bit baseline; next milestone is M3 memory management.

## Completed in this branch
- M1: GRUB Multiboot boot path loads a 32-bit protected-mode freestanding kernel and prints `Hello from kernel` to VGA and COM1 serial.
- M2: a minimal GDT is loaded, the IDT contains handlers for CPU exceptions 0-31, the PIC is remapped to IRQ vectors 32-47, and panic diagnostics print vector/error/EIP/register state before halting. Divide-error and page-fault tests are captured in `logs/m2_divide_error.log` and `logs/m2_page_fault.log`.
- M5 partial: kernel-local `memset`, `memcpy`, `strlen`, `strcmp`, and minimal formatted logging are implemented and smoke-tested at boot.
- M8 partial: an interactive serial shell supports `help`, `echo`, `clear`, `ls`, `reboot`, `status`, `test_div0`, and `test_pagefault`.

## Deferred / not yet implemented
- M3-M4, M6-M7, and M9-M50 remain deferred in this repository snapshot. The immediate next work is to turn the temporary identity paging setup into real M3 memory management, then add timer/keyboard interrupts (M4), then initrd filesystem content (M6).

## Architecture decisions
- Bootloader: GRUB legacy Multiboot v1 is used initially because it is readily testable with `grub-mkrescue` and QEMU and keeps early boot simple. Multiboot2/UEFI can be added later without replacing this working BIOS path.
- CPU mode: 32-bit protected mode at kernel entry, as provided by GRUB. Long mode is deferred until paging and higher-half design are established.
- Console: all kernel log output is mirrored to VGA text memory and COM1 serial. The automated QEMU test uses COM1 (`-serial stdio`) so boot evidence can be captured headlessly.

## Known bugs / limitations
- VGA scrolling is intentionally minimal; output pins at the last row instead of scrolling.
- The shell is serial-only for deterministic headless tests; PS/2 keyboard IRQ support is not implemented yet.
- Paging is currently only a single identity-mapped 4 MiB page table used to make page-fault tests meaningful; no allocator-backed M3 memory management exists yet.
- No filesystem, multitasking, storage, networking, graphics framebuffer, user mode, or advanced milestones are implemented yet.
