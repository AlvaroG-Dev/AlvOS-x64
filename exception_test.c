// exception_test.c - Pruebas intencionales de excepciones (CORREGIDO)
#include "terminal.h"
#include "panic.h"
#include "keyboard.h"
#include "timer.h"

// Variable global para almacenar la opción del usuario
static char test_option = 0;

// Provocar división por cero
void test_divide_by_zero(void) {
    terminal_writestring("\n=== TEST: Division por Cero ===\n");
    terminal_writestring("Provocando division por cero...\n");
    
    // Pequeña pausa para que el usuario pueda leer
    timer_wait(1000);
    
    // Esta línea causará una excepción
    volatile int x = 5;
    volatile int y = 0;
    volatile int z = x / y;
    (void)z;
    // No deberíamos llegar aquí
    terminal_writestring("ERROR: No se capturó la excepción!\n");
}

// Provocar opcode inválido
void test_invalid_opcode(void) {
    terminal_writestring("\n=== TEST: Opcode Invalido ===\n");
    terminal_writestring("Ejecutando instruccion UD2...\n");
    
    // Pequeña pausa para que el usuario pueda leer
    timer_wait(1000);
    
    // Instrucción UD2 - Opcode inválido
    __asm__ volatile ("ud2");
    
    // No deberíamos llegar aquí
    terminal_writestring("ERROR: No se capturó la excepción!\n");
}

// Provocar fallo de página
void test_page_fault(void) {
    terminal_writestring("\n=== TEST: Page Fault ===\n");
    terminal_writestring("Accediendo a direccion invalida...\n");
    
    // Pequeña pausa para que el usuario pueda leer
    timer_wait(1000);
    
    // Acceder a una dirección de memoria inválida
    volatile int *ptr = (int*)0xDEADBEEF;
    *ptr = 42;
    
    // No deberíamos llegar aquí
    terminal_writestring("ERROR: No se capturó la excepción!\n");
}

// Provocar general protection fault - CORREGIDO
void test_general_protection(void) {
    terminal_writestring("\n=== TEST: General Protection Fault ===\n");
    terminal_writestring("Cargando selector de segmento invalido...\n");
    
    // Pequeña pausa para que el usuario pueda leer
    timer_wait(1000);
    
    // Método 1: Cargar un selector de segmento inválido
    // Esto causará GPF de manera más confiable que intentar escribir en código
    __asm__ volatile (
        "mov $0xFFFF, %%ax\n"  // Selector inválido
        "mov %%ax, %%ds\n"     // Intentar cargar en DS
        : : : "ax"
    );
    
    // No deberíamos llegar aquí
    terminal_writestring("ERROR: No se capturó la excepción!\n");
}

// Stack fault - CORREGIDO con enfoque más agresivo
void test_stack_fault(void) {
    terminal_writestring("\n=== TEST: Stack Fault ===\n");
    terminal_writestring("Provocando desbordamiento de stack...\n");
    
    // Pequeña pausa para que el usuario pueda leer
    timer_wait(1000);
    
    // Método directo: acceder al stack pointer con offset masivo
    // Esto forzará un stack fault al intentar acceder más allá del límite
    __asm__ volatile (
        "mov %%rsp, %%rax\n"           // Guardar RSP actual
        "sub $0x100000, %%rsp\n"       // Mover RSP muy abajo (1MB)
        "pushq $0x12345678\n"          // Intentar push (causará stack fault)
        "mov %%rax, %%rsp\n"           // Restaurar RSP (no llegaremos aquí)
        : : : "rax", "rsp"
    );
    
    // No deberíamos llegar aquí
    terminal_writestring("ERROR: No se capturó la excepción!\n");
}

// Test alternativo para stack fault usando recursión descontrolada
// Mantenido como backup
static int recursive_depth = 0;
void recursive_overflow_simple(void) {
    // Buffer grande para consumir stack rápidamente
    volatile char buffer[4096];
    
    // Usar el buffer para que el compilador no lo optimice
    for (int i = 0; i < 4096; i++) {
        buffer[i] = (char)i;
    }
    
    recursive_depth++;
    
    // Mostrar progreso cada 100 llamadas
    if (recursive_depth % 100 == 0) {
        terminal_writestring("Profundidad: ");
        print_dec(recursive_depth);
        terminal_writestring("\n");
    }
    
    // Llamada recursiva sin fin
    recursive_overflow_simple();
}

void test_stack_fault_recursive(void) {
    terminal_writestring("\n=== TEST: Stack Fault (Recursivo) ===\n");
    terminal_writestring("Desbordamiento de stack por recursion infinita...\n");
    
    timer_wait(1000);
    
    recursive_depth = 0;
    recursive_overflow_simple();
    
    terminal_writestring("ERROR: No se capturó la excepción!\n");
}

// Función para ejecutar todas las pruebas de excepciones
void run_exception_tests(void) {
    terminal_writestring("\n=====================================\n");
    terminal_writestring("   PRUEBAS DE EXCEPCIONES DEL CPU\n");
    terminal_writestring("=====================================\n");
    terminal_writestring("ADVERTENCIA: Estas pruebas causarán crashes controlados\n");
    terminal_writestring("y reiniciarán el sistema. ¡Usar con precaución!\n\n");
    
    terminal_writestring("Selecciona una prueba:\n");
    terminal_writestring("1. Division por cero\n");
    terminal_writestring("2. Opcode invalido\n");
    terminal_writestring("3. Page fault\n");
    terminal_writestring("4. General protection fault\n");
    terminal_writestring("5. Stack fault (directo)\n");
    terminal_writestring("6. Stack fault (recursivo)\n");
    terminal_writestring("7. Cancelar\n");
    terminal_writestring("\nOpcion: ");
    
    // Resetear la opción
    test_option = 0;
    
    // Esperar input del usuario
    while (test_option == 0) {
        // Permitir que el timer y otras interrupciones sigan funcionando
        __asm__ volatile ("hlt");
        
        // Verificar si hay entrada del teclado
        if (keyboard_buffer_count() > 0) {
            char c = keyboard_getchar();
            if (c >= '1' && c <= '7') {
                test_option = c;
                terminal_putchar(c); // Mostrar la opción seleccionada
                terminal_writestring("\n");
            }
        }
    }
    
    terminal_writestring("\n");
    terminal_writestring("Ejecutando prueba en 2 segundos...\n");
    terminal_writestring("Presiona cualquier tecla para cancelar...\n");
    
    // Espera de 2 segundos con posibilidad de cancelar
    uint64_t start_ticks = timer_get_ticks();
    while ((timer_get_ticks() - start_ticks) < 200) { // 200 ticks = 2 segundos
        if (keyboard_buffer_count() > 0) {
            terminal_writestring("\nPrueba cancelada por el usuario.\n");
            return;
        }
        __asm__ volatile ("hlt");
    }
    
    terminal_writestring("\n");
    
    switch(test_option) {
        case '1':
            test_divide_by_zero();
            break;
        case '2':
            test_invalid_opcode();
            break;
        case '3':
            test_page_fault();
            break;
        case '4':
            test_general_protection();
            break;
        case '5':
            test_stack_fault();
            break;
        case '6':
            test_stack_fault_recursive();
            break;
        case '7':
            terminal_writestring("Pruebas canceladas.\n");
            break;
        default:
            terminal_writestring("Opcion invalida.\n");
            break;
    }
}

// Función para probar excepciones de forma automática (para debugging)
void run_quick_exception_test(uint8_t test_num) {
    terminal_writestring("\n=== PRUEBA RAPIDA DE EXCEPCION ===\n");
    
    switch(test_num) {
        case 0:
            test_divide_by_zero();
            break;
        case 1:
            test_invalid_opcode();
            break;
        case 2:
            test_page_fault();
            break;
        case 3:
            test_general_protection();
            break;
        case 4:
            test_stack_fault();
            break;
        case 5:
            test_stack_fault_recursive();
            break;
        default:
            terminal_writestring("Numero de prueba invalido.\n");
            break;
    }
}