// keyboard.c - Driver de teclado PS/2 (CORREGIDO - sin impresión automática)
#include "keyboard.h"
#include "terminal.h"
#include "isr.h"

// Scancode set 1 - Layout US
static const char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

// Scancode con Shift presionado
static const char scancode_to_ascii_shift[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

// Estado del teclado
static bool shift_pressed = false;
static bool ctrl_pressed = false;
static bool alt_pressed = false;
static bool caps_lock = false;

// Buffer circular para entrada de teclado
#define KEYBOARD_BUFFER_SIZE 256
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile size_t buffer_read_pos = 0;
static volatile size_t buffer_write_pos = 0;

// Port I/O
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Agregar carácter al buffer
static void keyboard_buffer_push(char c) {
    size_t next_pos = (buffer_write_pos + 1) % KEYBOARD_BUFFER_SIZE;
    if (next_pos != buffer_read_pos) {
        keyboard_buffer[buffer_write_pos] = c;
        buffer_write_pos = next_pos;
    }
}

// Obtener carácter del buffer (no bloqueante)
char keyboard_getchar_nonblock(void) {
    if (buffer_read_pos == buffer_write_pos) {
        return 0;  // Buffer vacío
    }
    
    char c = keyboard_buffer[buffer_read_pos];
    buffer_read_pos = (buffer_read_pos + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

uint8_t keyboard_buffer_count(void) {
    if (buffer_write_pos >= buffer_read_pos) {
        return buffer_write_pos - buffer_read_pos;
    } else {
        return KEYBOARD_BUFFER_SIZE - buffer_read_pos + buffer_write_pos;
    }
}

// Obtener carácter del buffer (bloqueante)
char keyboard_getchar(void) {
    while (buffer_read_pos == buffer_write_pos) {
        __asm__ volatile ("hlt");
    }
    
    char c = keyboard_buffer[buffer_read_pos];
    buffer_read_pos = (buffer_read_pos + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

// Handler del teclado (llamado desde IRQ1) - SIN IMPRIMIR AUTOMÁTICAMENTE
void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Detectar teclas especiales
    switch (scancode) {
        case 0x2A:  // Left Shift presionado
        case 0x36:  // Right Shift presionado
            shift_pressed = true;
            return;
        case 0xAA:  // Left Shift liberado
        case 0xB6:  // Right Shift liberado
            shift_pressed = false;
            return;
        case 0x1D:  // Ctrl presionado
            ctrl_pressed = true;
            return;
        case 0x9D:  // Ctrl liberado
            ctrl_pressed = false;
            return;
        case 0x38:  // Alt presionado
            alt_pressed = true;
            return;
        case 0xB8:  // Alt liberado
            alt_pressed = false;
            return;
        case 0x3A:  // Caps Lock
            caps_lock = !caps_lock;
            return;
    }
    
    // Ignorar key release events (scancode > 0x80)
    if (scancode & 0x80) {
        return;
    }
    
    // Convertir scancode a ASCII
    char ascii = 0;
    if (scancode < sizeof(scancode_to_ascii)) {
        if (shift_pressed) {
            ascii = scancode_to_ascii_shift[scancode];
        } else {
            ascii = scancode_to_ascii[scancode];
            // Aplicar Caps Lock solo a letras
            if (caps_lock && ascii >= 'a' && ascii <= 'z') {
                ascii -= 32;  // Convertir a mayúscula
            }
        }
        
        if (ascii != 0) {
            // SOLO AGREGAR AL BUFFER - NO IMPRIMIR AUTOMÁTICAMENTE
            // La impresión la maneja el shell
            keyboard_buffer_push(ascii);
        }
    }
}

// Inicializar el teclado
void keyboard_install(void) {
    // Habilitar IRQ1 (teclado)
    irq_clear_mask(1);
    
    terminal_writestring("[OK] Teclado PS/2 inicializado\n");
}

// Verificar si hay teclas en el buffer
bool keyboard_has_input(void) {
    return buffer_read_pos != buffer_write_pos;
}

// Limpiar el buffer
void keyboard_clear_buffer(void) {
    buffer_read_pos = 0;
    buffer_write_pos = 0;
}

// Obtener estado de teclas modificadoras
bool keyboard_shift_pressed(void) {
    return shift_pressed;
}

bool keyboard_ctrl_pressed(void) {
    return ctrl_pressed;
}

bool keyboard_alt_pressed(void) {
    return alt_pressed;
}

bool keyboard_caps_lock_active(void) {
    return caps_lock;
}