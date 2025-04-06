# Step 2 report

## Goals

The goal of this second week is to understand and implement interrupts to improve our echo console.

## 1 - File analysis

### isr-mmio.h

This isr.h file is a C header file that defines constants related to the Vectored Interrupt Controller (VIC)

### irq.s

This assembler file defines several functions for managing processor modes and interrupts, here is some detailled functions :

- \_wfi() : Puts the processor in interrupt wait mode to save power.
- \_irqs_setup() : Configure IRQ mode and initialize its stack

### isr.h & isr.c

- defines constants for interrupts (IRQs) on UART devices and timers.
- declares and implement functions to enable, disable, and configure interrupts on the Vectored Interrupt Controller (VIC).

## 2 - Implementing interrupts

### Registers & usefull constants

First, we need to know and define what registers and offsets will be needed to implement the interrupts. According to the documentation, i found the following :

- The base adress for the VIC :

```c
#define VIC_BASE_ADDR 0x10140000
```

- Shows the status of the interruptions (1 : active / 0 : inactive)

```c
#define VICFIQSTATUS 0x004
```

- VICINTENABLE register of the VIC, it is used to enable interrupts on the VIC, each bit corresponds to a specific interrupt, we will have to find the bit corresponding to the uart

```c
#define VICINTENABLE 0x010
```

- he maximum number of interrupts handled by the VIC

```c
#define NIRQS 32
```

- The mask to manipulate the bit that corresponds to the UART0 interrupt in the VICINTENABLE register

```c
#define UART0_IRQ 12
#define UART0_IRQ_MASK (1 << UART0_IRQ)
```

### Interrupt handling

#### Assembly level

- Isr call : In assembly exception file, we update the \_isr_handler to call the c level isr() function. The following lines save registers, call to isr function, the restore all registers

```
_isr_handler:
    sub lr, lr, #4
    stmfd sp!, {r0-r12, lr}
    bl isr
    ldmfd sp!, {r0-r12, pc}^
```

- Enabling IRQs : In startup file, we remove the CPSR_VIC_FLAG to ensure the interruptions are not disabled

```
msr     cpsr_c,#(CPSR_SYS_MODE | CPSR_FIQ_FLAG) # remove CPSR_VIC_FLAG
```

- Add IRQ stack memory: in kernel.ld, we need to add the following line to ensure there is memory allocation for the irq_stack

```
. = ALIGN(8);
. = . + 0x1000; /* 4KB of stack memory */
irq_stack_top = .;
```

#### C level

We firstly define a data structure and an array of handlers used to manage interrupt service routines (ISRs) :

```c
struct handler {
  void (*callback)(uint32_t, void*);
  void *cookie;
};

struct handler handlers[NIRQS];
```

The we can implement the interrupt service routine, it checks which interrupts are active and calls the appropriate handler :

```c
void isr() {
    const uint32_t irqs = mmio_read32((void *) VIC_BASE_ADDR, VICIRQSTATUS);;
    for (uint32_t i = 0; i < NIRQS; i++) {
      if (irqs & (1 << i) && handlers[i].callback) {
          handlers[i].callback(i, handlers[i].cookie);
      }
    }
}
```

- It read the VICIRQSTATUS wich contain all interrupts
- Loop through all possible interrupts (NIRQS = 32)
- If the interrupt is active (irqs & (1 << i)) and if there is a valid callback (handlers[i].callback), it call the appropriate callback

```c
void vic_disable_irq(uint32_t irq) {
    handlers[irq].callback = 0;
    handlers[irq].cookie = 0;
    mmio_write32((void *) VIC_BASE_ADDR, VICINTENABLE, 1 << irq);
}
```

This above funcion disables a specific interrupt in the VIC, by clearing the interrupt handler (first two lines), then by disabling the interrupt at the VIC level

```c
void vic_setup_irqs() {
    for (uint32_t i = 0; i < NIRQS; i++) {
      vic_disable_irq(i);
    }
    _irqs_setup();
}
```

The above function disable all interrupts at startup by calling vic_disable_irq() for each interrupt. then it call the assembly \_irqs_setup() to configure irq and irq' stack.

```c
void vic_enable_irq(uint32_t irq, void (*callback)(uint32_t, void*), void *cookie) {
    handlers[irq].callback = callback;
    handlers[irq].cookie = cookie;
    mmio_write32((void *) VIC_BASE_ADDR, VICINTENABLE, 1 << irq);
}
```

The last function enables a specific interrupt in the VIC:

- it stores the callback function (and cookie) in the handler's array
- then it enables the interrupt at the VIC level by writting in VICINTENABLE using the irq bitmask (1 << irq)

### Implementing the echo console in main

All that remain is to call our various elements in the main file, in the \_startup function:

- We initialize the UARTs and activate UART0
- We set up the IRQs
- We enable the interrupt for UART0 by passing it the uart_irq_handler() callback
- Finally, in an infinite loop, we wait for an interrupt using core_halt()

The uart_irq_handler() callback simply reads the character written to the UART and returns it

```c
void _start(void)
{
  check_stacks();

  uarts_init();
  uart_enable(UART0);

  vic_setup_irqs();
  vic_enable_irq(UART0_IRQ, uart_irq_handler, NULL);
  for (;;)
  {
    core_halt();
  }
}

void uart_irq_handler(uint32_t irq, void *cookie)
{
  char c;
  uart_receive(UART0, &c);
  uart_send(UART0, c);
}
```

## 3 - Character encoding observations

Serial communication sends data as bytes, not as individual characters or symbols.

Different keyboards and character sets encode symbols differently. By pressing some special keyboard key, the board receive the following :

- Rigth arrow : '\033[C' (3 characters)
- Bottom arrow : '\033[B' (3 characters)
- Left arrow : '\033[D' (3 characters)
- Top arrow : '\033[A' (3 characters)
- French accent "é" : '\303\251' (2 characters)
- Keyboard touch "Enter": '\r' (1 characters)
- Return : '\177' (1 characters)

In serial terminals, certain byte sequences are interpreted as commands instead of characters. For exemple "\033[H\033[J" clear the screen. It means that if I write this in the uart, the screen will be cleared. To test this, i add the following line in \_start() :

```c
  uart_send_string(UART0, "\033[H\033[J >");
```

The screen is cleared as espected.
