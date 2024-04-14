#pragma once

int uart_init();
char uart_read();
void uart_write(char c);
void uart_enable_irq();
void uart_intr();