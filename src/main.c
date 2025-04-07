/*
 * Copyright: Olivier Gruber (olivier dot gruber at acm dot org)
 *
 * This program is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */
#include "main.h"
#include "uart.h"
#include "isr.h"

extern uint32_t irq_stack_top;
extern uint32_t stack_top;

void check_stacks()
{
  void *memsize = (void *)MEMORY;
  void *addr;
  addr = &stack_top;
  if (addr >= memsize)
    panic();

  addr = &irq_stack_top;
  if (addr >= memsize)
    panic();
}

struct cookie cookie = {
    .uartno = UART0,
    .head = 0,
    .tail = 0,
    .processing = 0};

void write_listener(void *addr)
{
  struct cookie *cookie = addr;
  write_amap(cookie);
}

void write_amap(struct cookie *cookie)
{
  while (cookie->tail < cookie->head)
  {
    uint8_t code = cookie->line[cookie->tail];
    if (!uart_send(cookie->uartno, code))
      return;
    cookie->tail++;
  }
}

void read_listener(void *addr)
{
  struct cookie *cookie = (struct cookie *)addr;
  uint8_t code;
  while (!cookie->processing && uart_receive(cookie->uartno, &code))
  {
    cookie->line[cookie->head++] = (char)code;
    cookie->processing = (code == '\n');
    write_amap(cookie);
  }
  bool_t dropped = 0;
  while (cookie->processing && uart_receive(cookie->uartno, &code))
    dropped = 1;
  // if (dropped)
  //   panic();
}

/**
 * This is the C entry point,
 * upcalled once the hardware has been setup properly
 * in assembly language, see the startup.s file.
 */
void _start(void)
{
  check_stacks();
  uarts_init();
  uart_init(UART0, read_listener, write_listener, &cookie);

  uart_enable(UART0);
  uart_send_string(UART0, "\033[H\033[J >");
  vic_setup_irqs();
  // vic_enable_irq(UART0_IRQ, uart_irq_handler, NULL);
  for (;;)
  {
    core_halt();
  }
}

void panic()
{
  for (;;)
    ;
}
