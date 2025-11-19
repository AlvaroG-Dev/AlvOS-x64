// timer.c - Programmable Interval Timer (PIT) driver
#include "timer.h"
#include "terminal.h"
#include "isr.h"

static volatile uint64_t timer_ticks = 0;
static uint32_t timer_frequency = 0;

// Port I/O
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Callback del timer (llamado desde irq_handler)
void timer_handler(void) {
    timer_ticks++;
    
    // Mostrar tick cada segundo (si la frecuencia es 100Hz, cada 100 ticks = 1 segundo)
    if (timer_ticks % timer_frequency == 0) {
        //terminal_writestring("Timer: ");
        //print_dec(timer_ticks / timer_frequency);
        //terminal_writestring(" segundos\n");
    }
}

// Obtener el número de ticks desde el inicio
uint64_t timer_get_ticks(void) {
    return timer_ticks;
}

// Inicializar el PIT
void timer_install(uint32_t frequency) {
    timer_frequency = frequency;
    
    // El PIT corre a 1.193182 MHz
    uint32_t divisor = 1193182 / frequency;
    
    // Enviar el comando byte
    outb(0x43, 0x36);  // Canal 0, lobyte/hibyte, rate generator
    
    // Enviar divisor
    outb(0x40, divisor & 0xFF);         // Low byte
    outb(0x40, (divisor >> 8) & 0xFF);  // High byte
    
    // Habilitar IRQ0
    irq_clear_mask(0);
}

// Función de espera (busy wait)
void timer_wait(uint32_t milliseconds) {
    uint64_t target = timer_ticks + (milliseconds * timer_frequency / 1000);
    while (timer_ticks < target) {
        __asm__ volatile ("hlt");
    }
}