// keyboard.h - Declaraciones del driver de teclado
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

// Puertos del teclado PS/2
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_COMMAND_PORT 0x64

// Funciones del teclado
void keyboard_install(void);
void keyboard_handler(void);
char keyboard_getchar(void);
char keyboard_getchar_nonblock(void);
uint8_t keyboard_buffer_count(void);
bool keyboard_has_input(void);
void keyboard_clear_buffer(void);

// Estado de teclas modificadoras
bool keyboard_shift_pressed(void);
bool keyboard_ctrl_pressed(void);
bool keyboard_alt_pressed(void);
bool keyboard_caps_lock_active(void);

#endif