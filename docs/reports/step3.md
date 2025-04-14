# Step 2 report

Currently, our main function simply sleeps while waiting for an interrupt, and our handler's code does very little. But what if we wanted to do more in main, handle more interrupts, or if the handler's code grew larger and took longer to execute?

For exemple, if the interrupt handler takes too long to execute, another thread may access the variable before it is updated, leading to incorrect data. This is what we call race condition.

One solution could be to decouple the producer from the consumer by introducing a new data structure: ring buffers.

## 1 - Minimal rings buffer

Rings buffer are a kind of "FIFO" data structure. It works by allocating a chunk of sequential memory of a fixed length, then writing incoming data into the chunk, incrementing a counter with each write. When the counter reaches the end of the buffer, it goes around back to zero. As long as we don't exceed the buffer length, we don't have any problem.

First we have to implement the ring data structure, the code can be found [here](../../src/ring.h)

Next, in our uart_irq_handler, instead of reading an processing directly the data in the handler (wich can be long). The handler only put the readed bytes in the ring :

```c
void uart_irq_handler(uint32_t irq, void *cookie)
{
  char c;
  uart_receive(UART0, &c);

  while (c)
  {
    if (ring_full())
      panic();
    ring_put(c);
    uart_receive(UART0, &c);
  }
}
```

We also test if the ring is full before putting our character in the ring

Data processing is now done outside the interrupt, in the body of the main function:

```c
  for (;;)
  {
    process_ring();
    core_disable_irqs();
    if (ring_empty())
    {
      core_halt();
    }
    core_enable_irqs();
  }
```

By calling process_ring, which handles processing the contents of the ring buffer — in this case, by writing to the UART :

```c
void process_ring()
{
  uint8_t code;
  while (!ring_empty())
  {
    code = ring_get();
    line[nchars++] = (char)code;
    uart_send(UART0, code);
  }
}
```

## 2 - Using listeners

We want now to move to an event-driven model, where callbacks (listeners) are invoked as soon as an event occurs:

- Read listener: called as soon as bytes are available in the receive (RX) buffer.
- Write listener: called when there is space available to write in the transmit (TX) buffer.

To handle this, the idea is to use two ring buffers (one for reception and one for transmission) within a structure associated with the UART, along with an initialization function that allows you to provide the callbacks and a cookie (application-specific data).

### 1 - Init uart

We extends the uart structure by adding two rings (rx and tx) and two functions (read and write listeners) :

```c
struct uart
{
    struct ring rx; // reception buffer
    struct ring tx; // transmission buffer
    void (*read_listener)(void *cookie);
    void (*write_listener)(void *cookie);
    void *cookie;
    uint8_t uartno;
    void *bar;
};
```

Then we update the uart init function to take in consideration the new structure :

```c
void uart_init(uint8_t no,
               void (*rl)(void *cookie),
               void (*wl)(void *cookie),
               void *cookie,
               void *bar)
{
  struct uart *uart = &uarts[no];
  uart->uartno = no;
  uart->bar = bar;
  uart->read_listener = rl;
  uart->write_listener = wl;
  uart->cookie = cookie;

  ring_init(&uart->rx);
  ring_init(&uart->tx);

  uart_enable(no);
}
```

### 2 - irq handler :

Our irq handler doesn't change so much, we just retrieve uart from cookie and put received byte in rx ring

```c
void uart_irq_handler(void *cookie)
{
  struct uart *uart = (struct uart *)cookie;
  uint8_t code;

  uart_receive(uart->uartno, (char *)&code);

  while (code != '\0')
  {
    if (ring_full(&uart->rx))
      panic();
    ring_put(&uart->rx, code);

    uart_receive(uart->uartno, (char *)&code);
  }
}
```

### 3 - Listeners

We now have to implement our two listeners, the read listner will read in the tx ring, if it finds bytes, it will writes in the tx ring by calling write_amap() function.

```c
void read_listener(void *addr)
{
  struct cookie *cookie = (struct cookie *)addr;
  uint8_t code;
  while (!cookie->processing && uart_read(cookie->uartno, &code))
  {
    cookie->line[cookie->head++] = (char)code;
    cookie->processing = (code == '\n');
    write_amap(cookie);
  }
  bool_t dropped = 0;
  while (cookie->processing && uart_read(cookie->uartno, &code))
    dropped = 1;
  if (dropped)
    panic();
}
```

While the write listener will read the tx ring, and if there is available bytes it will send it to the uart

```c
void write_listener(void *addr)
{
  struct uart *uart = (struct uart *)addr;

  while (!ring_empty(&uart->tx))
  {
    uint8_t code = ring_get(&uart->tx);
    uart_send(uart->uartno, code);
  }
}
```

NB: I'm not sure about my write_listener usage, in the lecture provided in class, the write_listener is used to write in the tx ring

### 4 - Putting it all together in main & process rings

We now need to call our listners somewhere, for that, i created two functions: process_rx_ring and process_tx_ring :

```c
void process_rx_ring(struct uart *uart)
{
  if (!ring_empty(&uart->rx))
  {
    uart->read_listener(uart->cookie);
  }
}

void process_tx_ring(struct uart *uart)
{
  if (!ring_empty(&uart->tx))
  {
    uart->write_listener((void *)uart);
  }
}
```

process_rx_ring call to the uart read listener and process_tx_ring call to the write listener, both functions are called in process_uart() :

```c
void process_uart(uint8_t no)
{
  struct uart *uart = &uarts[no];
  process_rx_ring(uart);
  process_tx_ring(uart);
}
```

Then we jsut have to put all of this in our entry point :

```c
void _start(void)
{
  check_stacks();

  uart_init(UART0, read_listener, write_listener, &uart0_cookie, (void *)UART0_BASE_ADDRESS); // Setup our uart0 with write and read listeners, and cookie

  uart_send_string(UART0, "\033[H\033[J >");

  vic_setup_irqs();
  vic_enable_irq(UART0_IRQ, uart_irq_handler, &uarts[UART0]); // enable interruption for uart0

  for (;;)
  {
    core_disable_irqs();
    process_uart(UART0); // process tx and rx rings, then called each time an interrupt is raised
    core_halt(); // Wait for interrupt
    core_enable_irqs();
  }
}
```
