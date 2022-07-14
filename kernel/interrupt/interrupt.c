#include "interrupt/interrupt.h"
#include "common/x86.h"

struct idt_gate_desc idt[IRQ_CNT];

void idt_init() {}