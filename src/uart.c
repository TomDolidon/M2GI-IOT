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
#include "uart-mmio.h"

struct uart
{
  uint8_t uartno;
  void *bar;
  void (*read_listener)(void *cookie);
  void (*write_listener)(void *cookie);
  void *cookie;
};

static struct uart uarts[NUARTS];

static struct uart uarts[NUARTS];

static void uart_init_internal(uint8_t no, void *bar)
{
  struct uart *uart = &uarts[no];
  uart->uartno = no;
  uart->bar = bar;
  uart->read_listener = NULL;
  uart->write_listener = NULL;
  uart->cookie = NULL;
}

void uarts_init(void)
{
  uart_init_internal(UART0, (void *)UART0_BASE_ADDRESS);
  uart_init_internal(UART1, (void *)UART1_BASE_ADDRESS);
  uart_init_internal(UART2, (void *)UART2_BASE_ADDRESS);
}

void uart_init(uint8_t no,
               void (*read_listener)(void *cookie),
               void (*write_listener)(void *cookie),
               void *cookie)
{
  struct uart *uart = &uarts[no];
  uart->read_listener = read_listener;
  uart->write_listener = write_listener;
  uart->cookie = cookie;
}

/**
 * Enables the uart receive interrupt by setting the RXIM bit
 * in the Interrupt Mask Set/Clear Register (UART_IMSC).
 */
void uart_enable(uint32_t uartno)
{
  struct uart *uart = &uarts[uartno];
  mmio_write32(uart->bar, UART_IMSC, UART_IMSC_RXIM);
}

/**
 * Disables uart interrupts by clearing the UART_IMSC register.
 */
void uart_disable(uint32_t uartno)
{
  struct uart *uart = &uarts[uartno];
  mmio_write32(uart->bar, UART_IMSC, 0);
}

/**
 * Receives a character from the given UART and stores it in the given pointer.
 * Blocking call until a character is available in the UART's FIFO queue.
 */
bool_t uart_receive(uint8_t uartno, char *pt)
{
  struct uart *uart = &uarts[uartno];

  if (mmio_read8(uart->bar, UART_FR) & UART_RXFE)
  {
    *pt = '\0';
    return 0;
  }

  *pt = (char)mmio_read8(uart->bar, UART_DR);
  return 1;
}

/**
 * Receives a character from the given UART and stores it in the given pointer.
 * Blocking call until there is space in the  UART's FIFO queue
 */
bool_t uart_send(uint8_t uartno, char s)
{
  struct uart *uart = &uarts[uartno];
  if (mmio_read8(uart->bar, UART_FR) & UART_TXFF)
    return 0;
  mmio_write8(uart->bar, UART_DR, s);
  return 1;
}

/**
 * This is a wrapper function, provided for simplicity,
 * it sends a C string through the given uart.
 */
void uart_send_string(uint8_t uartno, const char *s)
{
  while (*s != '\0')
  {
    uart_send(uartno, *s);
    s++;
  }
}
