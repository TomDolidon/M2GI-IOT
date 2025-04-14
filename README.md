# M2 GI - IOT Course

This repository contains all the content produced as part of the IoT course in the Master 2 GI program at UGA.

This course is an introduction to bare metal development. To do this, we will simulate a board using QEMU.

You can follow the progress of the courses by viewing the reports [here](./docs/reports/)

## 🎉 Getting started

### 📀 Prerequisites

- QEMU for emulating ARM-based boards/sensors
- Cross-compilation GNU toolchain to compile from your Linux for our ARM-based sensors.
- GNU GDB multi-architecture.

#### Mac OS

Using Homebrew :

```console
brew install arm-none-eabi-gcc

brew install qemu
```

#### Debian and Ubuntu

Using apt :

```console
$ sudo apt install qemu-system-arm

$ sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi

$ sudo apt install gdb-multiarch
```

### ⏯️ Startup : Build and Run

At project root, build the code in a terminal :

```console
make
```

Launch project with :

```console
make run
```

## 🛠️ Development

### Debugging

Run QEMU with GDB debugging enabled:

```console
make debug
```

This will starts QEMU and waits for a GDB connection on port 1234.

In a new temrinal, run gdb :

```console
arm-none-eabi-gdb build/kernel.elf
```

Inside GDB, connect to QEMU:

```gdb
(gdb) target remote localhost:1234
```

## 🧱 Project structure

```
↳ 📄 README.md → this file
↳ 📄 Makefile -> handle project build, execution, debug and clean
↳ 📄 kernel.ld -> linker script used to organise how code will be stored in elf file
↳ 📄 .project
↳ 📄 .cproject
↳ 📂 .settings
↳ 📂 .vscode
↳ 📂 docs -> contain all documentation
↳ 📂 src
    ↳ 📄 exception.s -> exception handling for processor
    ↳ 📄 startup.s -> initialize the system then call _start() in main.c
    ↳ 📄 irq.s -> handle interrupt on the processor
    ↳ 📄 main.c -> code entry point
    ↳ 📄 main.h -> utility function to read and write at specific address
    ↳ 📄 uart-mmio.h -> constants for using uart
    ↳ 📄 uart.c & uart.h -> uart functions & definitions
    ↳ 📄 isr-mmio.h.h -> constants for using isr
    ↳ 📄 isr.c and isr.h -> isr function and definition
    ↳ 📄 ring.c and ring.h -> define ring structure and utility functions to use rings
```

## 📝 Documentation

- [Gdb cheat sheet](./docs/gdb_cheat_sheet.md)

## 🔗 Usefull links

- [QEMU documentation](https://www.qemu.org/docs/master/about/index.html)
- [GDB documentation](https://www.sourceware.org/gdb/documentation/)
