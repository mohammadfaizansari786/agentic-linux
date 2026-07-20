# Agentic OS

Agentic OS is a small x86 operating-system experiment. This snapshot establishes a bootable baseline from a clean checkout.

## What works now

- `./build.sh` produces `os.iso` using GRUB and a freestanding 32-bit Multiboot kernel.
- `./run-qemu.sh` boots the ISO in QEMU with `-serial stdio -monitor none -display none`.
- The kernel writes `Hello from kernel` to VGA text memory and COM1 serial.
- A minimal GDT, IDT, PIC remap, and exception panic path are installed.
- Shell-triggered `test_div0` and `test_pagefault` commands verify M2 diagnostics without triple-faulting.
- A serial shell reaches the `agentic-os>` prompt and supports:
  - `help`
  - `echo <text>`
  - `clear`
  - `ls`
  - `reboot` (halts in the current test build)
  - `status`
  - `test_div0`
  - `test_pagefault`

## Toolchain versions used

Captured on 2026-07-20 in this environment:

- GCC: `gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0` with `-m32 -ffreestanding`
- GNU ld: `GNU ld (GNU Binutils for Ubuntu) 2.42` with `-m elf_i386`
- NASM: installed and available for future assembly work, although this baseline uses GNU assembler syntax
- GRUB: `grub-mkrescue (GRUB) 2.12-1ubuntu7.3`
- QEMU: `QEMU emulator version 8.2.2 (Debian 1:8.2.2+ds-0ubuntu1.17)`
- xorriso: `xorriso 1.5.6`, used by `grub-mkrescue`

Run this to inspect exact local versions:

```sh
gcc --version
ld --version
grub-mkrescue --version
qemu-system-x86_64 --version
xorriso -version
```

## Build

```sh
./build.sh
```

The output ISO is `os.iso`.

## Run

```sh
./run-qemu.sh
```

For automated smoke testing, pipe commands into QEMU and terminate it with `timeout`, for example:

```sh
printf 'help\necho hi\nstatus\n' | timeout 5s ./run-qemu.sh
```

## Architecture notes

- Boot path: GRUB BIOS Multiboot v1 for the initial reliable baseline.
- Kernel mode: 32-bit protected mode as entered by GRUB.
- Logging: VGA text buffer plus COM1 serial; serial is the authoritative automated test output.
- Build system: one-command shell script; no manual ISO assembly steps.

## Milestone status

- M1: complete for this baseline (`Hello from kernel` is emitted and QEMU stays alive).
- M2: complete for this baseline (GDT/IDT/PIC setup plus captured divide-error and page-fault diagnostics).
- M5: partial (basic libc-like memory/string utilities and serial/VGA logging exist, with boot smoke tests).
- M8: partial (serial shell prompt and basic built-ins exist).
- M3-M4, M6-M7, M9-M50: deferred. See `STATE.md` for the current next-step plan and limitations.

## Extending

The recommended next implementation steps are:

1. Replace the temporary 4 MiB identity paging setup with M3 physical memory tracking and heap allocation.
2. Add PIT and PS/2 keyboard IRQ handling for M4.
3. Add a GRUB-loaded initrd/tarfs for M6.
4. Replace shell stubs with filesystem-backed commands.
