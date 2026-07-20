#!/usr/bin/env bash
set -euo pipefail
qemu-system-x86_64 -cdrom os.iso -serial stdio -monitor none -display none -no-reboot -m 128M "$@"
