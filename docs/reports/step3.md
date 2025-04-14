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
