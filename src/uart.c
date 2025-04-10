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
  uint8_t uartno; // the UART numéro
  void *bar;      // base address register for this UART
};

static struct uart uarts[NUARTS];

static void uart_init(uint32_t uartno, void *bar)
{
  struct uart *uart = &uarts[uartno];
  uart->uartno = uartno;
  uart->bar = bar;
  // no hardware initialization necessary
  // when running on QEMU, the UARTs are
  // already initialized, as long as we
  // do not rely on interrupts.
}

void uarts_init()
{
  uart_init(UART0, UART0_BASE_ADDRESS);
  uart_init(UART1, UART1_BASE_ADDRESS);
  uart_init(UART2, UART2_BASE_ADDRESS);
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
void uart_receive(uint8_t uartno, char *pt)
{
  struct uart *uart = &uarts[uartno];

  if (mmio_read8(uart->bar, UART_FR) & UART_RXFE)
  {
    *pt = '\0';
    return;
  }

  *pt = (char)mmio_read8(uart->bar, UART_DR);
}

/**
 * Receives a character from the given UART and stores it in the given pointer.
 * Blocking call until there is space in the  UART's FIFO queue
 */
void uart_send(uint8_t uartno, char s)
{
  struct uart *uart = &uarts[uartno];

  // while th fifo is full (UART_TXFF == 1), infinite loop
  while (mmio_read8(uart->bar, UART_FR) & UART_TXFF)
    ;

  // then write a character at uart data register adress
  mmio_write8(uart->bar, UART_DR, s);
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
