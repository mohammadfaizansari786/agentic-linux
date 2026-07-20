# Agentic OS Build Log

## 2026-07-20
- Bootstrapped the previously empty repository into a minimal GRUB-loaded hobby OS.
- Added a freestanding 32-bit Multiboot kernel entry point, linker script, C kernel, build script, QEMU runner, and project documentation.
- Installed QEMU/GRUB/xorriso/mtools in the container so the ISO can be built and boot-tested.
- Implemented VGA + COM1 serial logging and a serial shell prompt so `./run-qemu.sh` reaches an interactive prompt under headless QEMU.
- Added boot-time utility smoke tests for `memset`, `memcpy`, `strlen`, and `strcmp`.
- Captured passing build and boot output in `logs/m1_m5_m8_boot.log`.
- Fixed the QEMU runner to use `-monitor none -display none` with `-serial stdio`; this avoids stdio contention in headless runs.
- Rebuilt after adding a non-executable stack note to the assembly entry file; `./build.sh` now completes without linker warnings.

- Implemented M2 core CPU infrastructure: a known GDT, IDT exception gates for vectors 0-31, 8259 PIC remapping to 0x20/0x28, and panic diagnostics with register state.
- Added a temporary 4 MiB identity-mapped paging setup so the page-fault acceptance test can produce a real #PF diagnostic before full M3 memory management exists.
- Added shell commands `test_div0` and `test_pagefault` and captured passing serial logs in `logs/m2_divide_error.log` and `logs/m2_page_fault.log`.
