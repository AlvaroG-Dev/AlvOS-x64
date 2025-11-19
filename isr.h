// isr.h - Declaraciones para el sistema de interrupciones
#ifndef ISR_H
#define ISR_H

#include <stdint.h>
#include "constants.h"

// Comandos de inicialización del PIC
#define ICW1_ICW4       0x01    // ICW4 needed
#define ICW1_SINGLE     0x02    // Single (cascade) mode
#define ICW1_INTERVAL4  0x04    // Call address interval 4 (8)
#define ICW1_LEVEL      0x08    // Level triggered (edge) mode
#define ICW1_INIT       0x10    // Initialization

#define ICW4_8086       0x01    // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO       0x02    // Auto (normal) EOI
#define ICW4_BUF_SLAVE  0x08    // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C    // Buffered mode/master
#define ICW4_SFNM       0x10    // Special fully nested (not)

// Funciones del sistema de interrupciones
void isr_install(void);
void pic_remap(void);
void pic_send_eoi(uint8_t irq);
void irq_set_mask(uint8_t irq);
void irq_clear_mask(uint8_t irq);

// Funciones de la IDT (definidas en kernel.c)
void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t flags);

// Funciones del terminal (definidas en terminal.c)
void terminal_writestring(const char* data);
void print_hex(uint64_t value);

#endif