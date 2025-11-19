// panic.c - Sistema de pánico para errores fatales
#include "panic.h"
#include "terminal.h"

// Detener el sistema completamente
static void halt_system(void) {
    __asm__ volatile ("cli");  // Deshabilitar interrupciones
    while (1) {
        __asm__ volatile ("hlt");
    }
}

// Función panic - error fatal irrecuperable
void kernel_panic(const char* message, const char* file, int line) {
    // Deshabilitar interrupciones inmediatamente
    __asm__ volatile ("cli");
    
    // Limpiar pantalla y mostrar mensaje de pánico
    terminal_initialize();
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    
    terminal_writestring("\n");
    terminal_writestring("=====================================\n");
    terminal_writestring("       KERNEL PANIC!                 \n");
    terminal_writestring("=====================================\n");
    terminal_writestring("\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    terminal_writestring("Error fatal: ");
    terminal_writestring(message);
    terminal_writestring("\n\n");
    
    if (file) {
        terminal_writestring("Archivo: ");
        terminal_writestring(file);
        terminal_writestring("\n");
    }
    
    if (line >= 0) {
        terminal_writestring("Linea: ");
        print_dec(line);
        terminal_writestring("\n");
    }
    
    terminal_writestring("\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("El sistema se ha detenido para prevenir danos.\n");
    terminal_writestring("Presione RESET para reiniciar.\n");
    
    // Detener el sistema
    halt_system();
}

// Assert para debugging
void kernel_assert_fail(const char* expression, const char* file, int line) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    terminal_writestring("\n[ASSERT FAILED] ");
    terminal_writestring(expression);
    terminal_writestring("\n");
    
    kernel_panic("Assertion failed", file, line);
}