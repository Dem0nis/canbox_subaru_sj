#!/bin/bash
qemu-system-arm -cpu cortex-m3 -M stm32vldiscovery -semihosting -kernel qemu.bin

