#include "dev/uart.h"
#include "common/x86.h"
#include "dev/console.h"
#include "interrupt/interrupt.h"

#define PORT 0x3F8

int uart_init() {
  outb(PORT + 1, 0x00);  // Disable all interrupts
  outb(PORT + 3, 0x80);  // Enable DLAB (set baud rate divisor)
  outb(PORT + 0, 0x03);  // Set divisor to 3 (lo byte) 38400 baud
  outb(PORT + 1, 0x00);  //                  (hi byte)
  outb(PORT + 3, 0x03);  // 8 bits, no parity, one stop bit
  outb(PORT + 2, 0xC7);  // Enable FIFO, clear them, with 14-byte threshold
  outb(PORT + 4, 0x0B);  // IRQs enabled, RTS/DSR set
  outb(PORT + 4, 0x1E);  // Set in loopback mode, test the serial chip
  outb(PORT + 0, 0xAE);  // Test serial chip (send byte 0xAE and check if serial
                         // returns same byte)

  // Check if serial is faulty (i.e: not same byte as sent)
  if (inb(PORT + 0) != 0xAE) {
    return 1;
  }

  // If serial is not faulty set it in normal operation mode
  // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
  outb(PORT + 4, 0x0F);
  return 0;
}

char uart_read() {
  if ((inb(PORT + 5) & 0x1) == 0)
    return CONSOLE_READ_INVALID;
  return inb(PORT);
}

void uart_write(char c) {
  while ((inb(PORT + 5) & 0x20) == 0)
    ;
  outb(PORT, c);
}

void uart_enable_irq() {
  outb(PORT + 1, 0x01);  // Enable receive data available interrupt
  ioapic_enable(IRQ_DEV_UART, 0);
}

void uart_intr() {
  consoleintr(uart_read);
}