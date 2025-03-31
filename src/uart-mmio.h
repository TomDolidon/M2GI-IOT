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

#ifndef UART_MMIO_H_
#define UART_MMIO_H_

/**
 * To fill this header file,
 * look at the document describing the Versatile Application Board:
 *
 *    Versatile Application Baseboard for ARM926EJ-S (DUI0225)
 *    doc DUI0225D_versatile_application_baseboard_arm926ej_s_ug-1.pdf page 140
 */

/*
 * We need the base address for the different serial lines.
 */

#define UART0_BASE_ADDRESS ((void*)0x101F1000) // -> 0x101F1FFF 4KB
#define UART1_BASE_ADDRESS ((void*)0x101F2000) // -> 0x101F2FFF 4KB
#define UART2_BASE_ADDRESS ((void*)0x101F3000) // -> 0x101F3FFF 4KB

/*
 * Is the UART chipset a PL011?
 * If so, we need the details for the data and status registers.
 * doc DUI0225D_versatile_application_baseboard_arm926ej_s_ug-1.pdf page 202 : it seems UART chipset is a PL011
 * doc DDI0183G-UART_PL011_r1p5_TRM.pdf p. 48
 */
#define UART_DR 0x000 // data register 
#define UART_FR 0x018 // flag register or status register 
#define UART_IMSC 0x038 // interrupt mask set/clear register

#define UART_RXFE (1 << 4)  // FIFO Empty
#define UART_TXFF (1 << 5)  // FIFO Full

#define UART_IMSC_RXIM (1 << 4) // receive interrupt mask 
#define UART_IMSC_TXIM (1 << 5) // transmit interrupt mask

#endif /* UART_MMIO_H_ */
