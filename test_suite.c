// test_suite.c - Suite de pruebas para el kernel
#include "test_suite.h"
#include "terminal.h"
#include "timer.h"
#include "keyboard.h"
#include "memory_tests.h"
#include "exception_test.h"

// Colores para los tests
#define TEST_COLOR_PASS vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK)
#define TEST_COLOR_FAIL vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK)
#define TEST_COLOR_INFO vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK)
#define TEST_COLOR_WARN vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK)

static int tests_passed = 0;
static int tests_failed = 0;

// Utilidades de testing
void test_print_pass(const char* test_name) {
    terminal_setcolor(TEST_COLOR_PASS);
    terminal_writestring("[PASS] ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring(test_name);
    terminal_writestring("\n");
    tests_passed++;
}

void test_print_fail(const char* test_name) {
    terminal_setcolor(TEST_COLOR_FAIL);
    terminal_writestring("[FAIL] ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring(test_name);
    terminal_writestring("\n");
    tests_failed++;
}

void test_print_info(const char* message) {
    terminal_setcolor(TEST_COLOR_INFO);
    terminal_writestring("[INFO] ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring(message);
    terminal_writestring("\n");
}

void test_print_warn(const char* message) {
    terminal_setcolor(TEST_COLOR_WARN);
    terminal_writestring("[WARN] ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring(message);
    terminal_writestring("\n");
}

// Test 1: Timer básico
void test_timer_basic(void) {
    terminal_writestring("\n=== TEST: Timer Basico ===\n");
    
    uint64_t start_ticks = timer_get_ticks();
    test_print_info("Esperando 3 segundos...");
    
    timer_wait(3000);  // Esperar 3 segundos
    
    uint64_t end_ticks = timer_get_ticks();
    uint64_t elapsed = end_ticks - start_ticks;
    
    // Debería ser aproximadamente 300 ticks (100Hz * 3 segundos)
    if (elapsed >= 290 && elapsed <= 310) {
        test_print_pass("Timer cuenta correctamente (3 segundos)");
    } else {
        test_print_fail("Timer no cuenta correctamente");
        terminal_writestring("  Esperado: ~300 ticks, Obtenido: ");
        print_dec(elapsed);
        terminal_writestring(" ticks\n");
    }
}

// Test 2: Timer overflow check
void test_timer_overflow(void) {
    terminal_writestring("\n=== TEST: Timer Overflow Check ===\n");
    
    uint64_t ticks = timer_get_ticks();
    test_print_info("Ticks actuales: ");
    print_dec(ticks);
    terminal_writestring("\n");
    
    if (ticks < 0xFFFFFFFFFFFFFFFF) {
        test_print_pass("Timer no ha hecho overflow");
    } else {
        test_print_warn("Timer cerca del overflow");
    }
}

// Test 3: Teclado - letras minúsculas
void test_keyboard_lowercase(void) {
    terminal_writestring("\n=== TEST: Teclado - Letras Minusculas ===\n");
    test_print_info("Escribe el abecedario en minusculas y presiona Enter:");
    terminal_writestring("> ");
    
    keyboard_clear_buffer();
    
    char buffer[27];
    int i = 0;
    while (i < 26) {
        char c = keyboard_getchar();
        if (c == '\n') break;
        buffer[i++] = c;
    }
    buffer[i] = '\0';
    
    // Verificar que son letras minúsculas
    bool all_lowercase = true;
    for (int j = 0; j < i; j++) {
        if (buffer[j] < 'a' || buffer[j] > 'z') {
            all_lowercase = false;
            break;
        }
    }
    
    if (all_lowercase && i > 0) {
        test_print_pass("Letras minusculas recibidas correctamente");
    } else {
        test_print_fail("Problema con letras minusculas");
    }
}

// Test 4: Teclado - mayúsculas con Shift
void test_keyboard_shift(void) {
    terminal_writestring("\n=== TEST: Teclado - Shift ===\n");
    test_print_info("Escribe 5 letras MAYUSCULAS (usando Shift) y presiona Enter:");
    terminal_writestring("> ");
    
    keyboard_clear_buffer();
    
    char buffer[10];
    int i = 0;
    while (i < 5) {
        char c = keyboard_getchar();
        if (c == '\n') break;
        buffer[i++] = c;
    }
    buffer[i] = '\0';
    
    bool all_uppercase = true;
    for (int j = 0; j < i; j++) {
        if (buffer[j] < 'A' || buffer[j] > 'Z') {
            all_uppercase = false;
            break;
        }
    }
    
    if (all_uppercase && i > 0) {
        test_print_pass("Shift funciona correctamente");
    } else {
        test_print_fail("Problema con Shift");
    }
}

// Test 5: Teclado - números
void test_keyboard_numbers(void) {
    terminal_writestring("\n=== TEST: Teclado - Numeros ===\n");
    test_print_info("Escribe los numeros del 0 al 9 y presiona Enter:");
    terminal_writestring("> ");
    
    keyboard_clear_buffer();
    
    char buffer[11];
    int i = 0;
    while (i < 10) {
        char c = keyboard_getchar();
        if (c == '\n') break;
        buffer[i++] = c;
    }
    buffer[i] = '\0';
    
    bool all_numbers = true;
    for (int j = 0; j < i; j++) {
        if (buffer[j] < '0' || buffer[j] > '9') {
            all_numbers = false;
            break;
        }
    }
    
    if (all_numbers && i > 0) {
        test_print_pass("Numeros funcionan correctamente");
    } else {
        test_print_fail("Problema con numeros");
    }
}

// Test 6: Backspace
void test_keyboard_backspace(void) {
    terminal_writestring("\n=== TEST: Teclado - Backspace ===\n");
    test_print_info("Escribe 'ERROR' y luego borra todo con Backspace:");
    terminal_writestring("> ");
    
    keyboard_clear_buffer();
    
    // Esperar input
    test_print_info("Presiona Enter cuando termines de borrar");
    while (keyboard_getchar() != '\n') {
        // Esperar Enter
    }
    
    test_print_pass("Backspace probado (verificacion visual)");
}

// Test 7: Terminal - colores
void test_terminal_colors(void) {
    terminal_writestring("\n=== TEST: Terminal - Colores ===\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_BLUE, VGA_COLOR_BLACK));
    terminal_writestring("Azul ");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Verde ");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_RED, VGA_COLOR_BLACK));
    terminal_writestring("Rojo ");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    terminal_writestring("Amarillo ");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("Magenta ");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("Cian\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    
    test_print_pass("Colores mostrados (verificacion visual)");
}

// Test 8: Terminal - scroll
void test_terminal_scroll(void) {
    terminal_writestring("\n=== TEST: Terminal - Scroll ===\n");
    test_print_info("Llenando pantalla para probar scroll...");
    
    for (int i = 0; i < 30; i++) {
        terminal_writestring("Linea ");
        print_dec(i);
        terminal_writestring(" - Probando scroll del terminal\n");
        timer_wait(50);  // Pequeña pausa para ver el scroll
    }
    
    test_print_pass("Scroll probado (verificacion visual)");
}

// Test 9: Excepciones del CPU (interactivo)
void test_exceptions_interactive(void) {
    terminal_writestring("\n=== TEST: Excepciones del CPU ===\n");
    test_print_info("Este test es interactivo y requiere seleccion manual");
    test_print_warn("¡ADVERTENCIA: Causara crash y reinicio del sistema!");
    
    terminal_writestring("\n¿Ejecutar pruebas de excepciones? (s/n): ");
    
    char response = keyboard_getchar();
    terminal_writestring("\n");
    
    if (response == 's' || response == 'S') {
        run_exception_tests();
        test_print_pass("Pruebas de excepciones completadas");
    } else {
        test_print_info("Pruebas de excepciones omitidas");
    }
}

// Resumen de tests
void test_print_summary(void) {
    terminal_writestring("\n");
    terminal_writestring("=====================================\n");
    terminal_writestring("       RESUMEN DE TESTS\n");
    terminal_writestring("=====================================\n");
    
    terminal_setcolor(TEST_COLOR_PASS);
    terminal_writestring("Tests pasados: ");
    print_dec(tests_passed);
    terminal_writestring("\n");
    
    if (tests_failed > 0) {
        terminal_setcolor(TEST_COLOR_FAIL);
        terminal_writestring("Tests fallidos: ");
        print_dec(tests_failed);
        terminal_writestring("\n");
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("Total: ");
    print_dec(tests_passed + tests_failed);
    terminal_writestring("\n");
    
    if (tests_failed == 0) {
        terminal_setcolor(TEST_COLOR_PASS);
        terminal_writestring("\n¡Todos los tests pasaron! ✓\n");
    } else {
        terminal_setcolor(TEST_COLOR_WARN);
        terminal_writestring("\nAlgunos tests fallaron.\n");
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

// Ejecutar suite completa
void run_test_suite(void) {
    terminal_initialize();
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("=====================================\n");
    terminal_writestring("   SUITE DE PRUEBAS DEL KERNEL\n");
    terminal_writestring("=====================================\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    test_exceptions_interactive();
    // Tests automáticos
    test_timer_basic();
    test_timer_overflow();
    
    // Tests interactivos
    test_keyboard_numbers();
    test_keyboard_backspace();
    
    // Tests visuales
    test_terminal_colors();
    test_terminal_scroll();
    
    run_memory_tests();

    // Resumen
    test_print_summary();
}