# Step 1 report

## Goals

The objective of this first step is twofold:

- Analyze and understand the various provided files.
- Get the UART0 to work by providing an “echo console

## 1 - File analysis

### Makefile

The makefile is doing several things :

- L17-22: Defines config parameters: the board type (versatile), the CPU (cortex-a8), and TOOLCHAIN (arm-none-eabi).
- L24: Specifies compiled files (startup.o, main.o, etc).
- L32-42: Set up QEMU configuration for versatile board (eg: memory size)
- L44-54: Set up compilation flages, including debug
- L63-67: Provide compiing rules using the defined toolchain
- L73-85: Define build process, linking object file using kernel.ld to kernel.elf(\*) and converting it to kernel.bin
- L84-85: Includes a cleanup command
- L90-98: Implement QEMU execution (run and debug)

(\*) An ELF file (Executable and Linkable Format) is a file format used for executables, shared libraries, and object files on Unix/Linux and embedded systems, it contains all necessary metadata for execution and debugging. It is finally converted to .bin for direct execution on the target.

### kernel.ld

The .ld format (linker script) is used to organise how code will be stored in elf file. I haven't looked at the lines in detail yet.

### Source code

#### main.c

The main C file, it currently contains 2 functions :

- check_stacks() wich check if the stack adress doesn't exceed the allocated memory.
- \_start() is the programm entry point, this function do the following :
  - check the stack
  - enable and init uart0
  - in infinite loop, try to send and receive from uart0

#### main.h

Define severals multiple functions to handle read and write to memory at specific addresses (here to MMIO(\*)).

(\*) MMIO (Memory-Mapped I/O) allows CPUs to interact with hardware using standard memory operations: each device has a mapped address range where registers can be read or written. It is used in embedded systems to control peripherals like UART.

#### uart-mmio.h

Provide the necessary configurations and definitions to interact with the UART using mmio on the versatile board. The file defines :

- Base addresses for UART peripherals (eg: UART0_BASE_ADDRESS)
- Data register and flag register address
- Defines flags masks for UART status

#### uart.c and uart.h

This files defines a set of functions to interact with UART devices using MMIO.

#### exception.s

This assembly code sets up exception handling for the processor.

#### startup.s

This assembly file initializes the system and then call the C entry function \_start().

## 2 - Making UART to work

When a keyboard key is pressed, a signal is sent through a serial line to the UART. The UART will retrieve the signal bit by bit and translate it into an ASCII character, then store it in its data register.

To enable our "echo", we will need to:

- read the characters in this data register
- return these characters to the data register (because it's an MMIO)

First, we need to know some specific adress for our uart (by checking the board documentation):

- the uart base adress
- the uart data register offste
- the UART_RXFE flag to knwo if the stack is empty
- the UART_TXFF flag to knwo if the stack is full
- the uart flags offset to retrieve the above flags

Then, we can implement our receive and send function :

```c
/**
 * Receives a character from the given UART and stores it in the given pointer.
 * Blocking call until a character is available in the UART's FIFO queue.
*/
void uart_receive(uint8_t uartno, char *pt) {
    // Retrieve the uart struc to get the given uart adress
    struct uart* uart = &uarts[uartno];

    // while no characters are available (FIFO Empty, UART_RXFE == 1), infinite loop
    while (mmio_read8(uart->bar, UART_FR) & UART_RXFE)
        ;

    // read in the uart data register adress the character and store it at pt adress
    *pt = (char)mmio_read8(uart->bar, UART_DR);
}

/**
 * Receives a character from the given UART and stores it in the given pointer.
 * Blocking call until there is space in the  UART's FIFO queue
*/
void uart_send(uint8_t uartno, char s) {
    struct uart* uart = &uarts[uartno];

    // while th fifo is full (UART_TXFF == 1), infinite loop
    while (mmio_read8(uart->bar, UART_FR) & UART_TXFF)
        ;

    // then write a character at uart data register adress
    mmio_write8(uart->bar,UART_DR, s);
}
```
