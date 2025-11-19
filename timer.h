// timer.h - Declaraciones del driver PIT
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Funciones del timer
void timer_install(uint32_t frequency);
void timer_handler(void);
uint64_t timer_get_ticks(void);
void timer_wait(uint32_t milliseconds);

#endif