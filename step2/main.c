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
#include "isr.h"

extern uint32_t irq_stack_top;
extern uint32_t stack_top;

void check_stacks() {
  void *memsize = (void*)MEMORY;
  void *addr;
  addr = &irq_stack_top;
  if (addr >= memsize)
    panic();
}

/*
 * Interrupts handlers
 */
void uart0_interrupt_handler(uint32_t vicirq, void* cookie) {
  // vicirq is always UART0_IRQ, it's not a UART interrupt
  char c;
  uint32_t irqs = mmio_read32((void *)UART0_BASE_ADDRESS, UART_MIS);
  if (irqs & UART_IMSC_RXIM) {
    uart_receive(UART0, &c);
    uart_send(UART0, c);
    // no ACK necessary
    // mmio_set(UART0_BASE_ADDRESS, UART_ICR, (1 << UART_IMSC_RXIM));
  }
  else {
    panic();
  }
}

/**
 * This is the C entry point,
 * upcalled once the hardware has been setup properly
 * in assembly language, see the startup.s file.
 */
void _start(void) {
  check_stacks(); 
  uarts_init();
  uart_enable(UART0);
  vic_setup_irqs();
  vic_enable_irq(UART0_IRQ, &uart0_interrupt_handler, NULL);
  core_enable_irqs();
  for (;;) {
    core_halt();
  }
}

void panic() {
  for(;;)
    ;
}
