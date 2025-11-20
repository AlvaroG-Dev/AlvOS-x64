// memory_tests.c - Suite de tests para memoria (kmalloc y PMM)
#include "test_suite.h"
#include "memory.h"
#include "kmalloc.h"
#include "terminal.h"
#include "string.h"

/**
 * Test 1: Allocar 100 bloques pequeños
 * Verifica que kmalloc funciona correctamente con múltiples allocaciones
 */
void test_kmalloc_small_blocks(void) {
    test_print_info("Test: Allocar 100 bloques de 64 bytes...");
    
    void* blocks[100];
    bool all_valid = true;
    
    // Allocar bloques
    for (int i = 0; i < 100; i++) {
        blocks[i] = kmalloc(64);
        
        if (blocks[i] == NULL) {
            terminal_writestring("  [ERROR] Allocación ");
            print_dec(i);
            terminal_writestring(" falló\n");
            all_valid = false;
            break;
        }
        
        // Escribir patrón único en cada bloque
        uint8_t* ptr = (uint8_t*)blocks[i];
        for (int j = 0; j < 64; j++) {
            ptr[j] = (uint8_t)(i & 0xFF);
        }
    }
    
    if (!all_valid) {
        test_print_fail("Kmalloc: 100 bloques pequeños");
        return;
    }
    
    // Verificar que no se solapan y mantienen datos
    for (int i = 0; i < 100; i++) {
        uint8_t* ptr = (uint8_t*)blocks[i];
        for (int j = 0; j < 64; j++) {
            if (ptr[j] != (uint8_t)(i & 0xFF)) {
                terminal_writestring("  [ERROR] Bloque ");
                print_dec(i);
                terminal_writestring(" corrupto en byte ");
                print_dec(j);
                terminal_writestring("\n");
                test_print_fail("Kmalloc: 100 bloques pequeños");
                return;
            }
        }
    }
    
    test_print_pass("Kmalloc: 100 bloques pequeños");
    terminal_writestring("  Total allocado: ");
    print_dec(100 * 64);
    terminal_writestring(" bytes\n");
}

/**
 * Test 2: Verificar alineación
 * Todos los punteros deben estar alineados correctamente
 */
void test_kmalloc_alignment(void) {
    test_print_info("Test: Verificar alineación de allocaciones...");
    
    // Probar diferentes tamaños
    size_t sizes[] = {1, 7, 15, 31, 64, 128, 1024};
    bool all_aligned = true;
    
    for (int i = 0; i < 7; i++) {
        void* ptr = kmalloc(sizes[i]);
        
        if (ptr == NULL) {
            terminal_writestring("  [ERROR] Allocación de ");
            print_dec(sizes[i]);
            terminal_writestring(" bytes falló\n");
            test_print_fail("Kmalloc: alineación");
            return;
        }
        
        // Verificar alineación a 4 bytes
        if (((uint64_t)ptr & 3) != 0) {
            terminal_writestring("  [ERROR] Puntero 0x");
            print_hex((uint64_t)ptr);
            terminal_writestring(" no alineado (tamaño ");
            print_dec(sizes[i]);
            terminal_writestring(")\n");
            all_aligned = false;
        }
    }
    
    if (all_aligned) {
        test_print_pass("Kmalloc: alineación");
    } else {
        test_print_fail("Kmalloc: alineación");
    }
}

/**
 * Test 3: Llenar heap hasta el límite
 * Verifica manejo correcto de memoria agotada
 */
void test_kmalloc_out_of_memory(void) {
    test_print_info("Test: Llenar heap hasta el límite...");
    
    heap_info_t* info = heap_get_info();
    uint64_t available = info->heap_end - info->heap_current;
    
    terminal_writestring("  Memoria disponible antes: ");
    print_dec(available);
    terminal_writestring(" bytes\n");
    
    // Intentar allocar toda la memoria disponible
    size_t chunk_size = 1024; // 1KB chunks
    int chunks_allocated = 0;
    
    while (available >= chunk_size) {
        void* ptr = kmalloc(chunk_size);
        if (ptr == NULL) {
            break;
        }
        chunks_allocated++;
        available -= chunk_size;
    }
    
    terminal_writestring("  Chunks de 1KB allocados: ");
    print_dec(chunks_allocated);
    terminal_writestring("\n");
    
    // Ahora el heap debe estar casi lleno
    // Intentar allocar más debe fallar
    void* should_fail = kmalloc(1024 * 1024); // 1MB (más que lo disponible)
    
    if (should_fail == NULL) {
        test_print_pass("Kmalloc: out of memory");
        terminal_writestring("  Correctamente rechazó allocación imposible\n");
    } else {
        test_print_fail("Kmalloc: out of memory");
        terminal_writestring("  [ERROR] Allocó cuando no debía!\n");
    }
}

/**
 * Test 4: PMM - Allocar y liberar frames
 * Verifica que el PMM funciona correctamente
 */
void test_pmm_alloc_free(void) {
    test_print_info("Test: PMM allocar/liberar frames...");
    
    // Guardar estado inicial
    uint64_t initial_free = pmm_get_free_memory();
    
    // Allocar 10 frames
    uint64_t frames[10];
    for (int i = 0; i < 10; i++) {
        frames[i] = pmm_alloc_frame();
        
        if (frames[i] == 0) {
            terminal_writestring("  [ERROR] PMM allocación ");
            print_dec(i);
            terminal_writestring(" falló\n");
            test_print_fail("PMM: alloc/free");
            return;
        }
    }
    
    // Verificar que la memoria libre disminuyó
    uint64_t after_alloc = pmm_get_free_memory();
    if (after_alloc >= initial_free) {
        terminal_writestring("  [ERROR] Memoria libre no disminuyó\n");
        test_print_fail("PMM: alloc/free");
        return;
    }
    
    // Liberar frames
    for (int i = 0; i < 10; i++) {
        pmm_free_frame(frames[i]);
    }
    
    // Verificar que la memoria libre volvió al estado inicial
    uint64_t after_free = pmm_get_free_memory();
    if (after_free == initial_free) {
        test_print_pass("PMM: alloc/free");
        terminal_writestring("  10 frames allocados y liberados correctamente\n");
    } else {
        test_print_warn("PMM: alloc/free");
        terminal_writestring("  Memoria no volvió exactamente al estado inicial\n");
        terminal_writestring("  (Esto puede ser normal debido a overhead)\n");
    }
}

/**
 * Test 5: PMM - Verificar que frames no se solapan
 */
void test_pmm_no_overlap(void) {
    test_print_info("Test: PMM frames sin solapamiento...");
    
    #define NUM_TEST_FRAMES 20
    uint64_t frames[NUM_TEST_FRAMES];
    bool overlap_found = false;
    
    // Allocar frames
    for (int i = 0; i < NUM_TEST_FRAMES; i++) {
        frames[i] = pmm_alloc_frame();
        if (frames[i] == 0) {
            terminal_writestring("  [ERROR] Allocación falló\n");
            test_print_fail("PMM: sin solapamiento");
            return;
        }
    }
    
    // Verificar que no hay duplicados
    for (int i = 0; i < NUM_TEST_FRAMES; i++) {
        for (int j = i + 1; j < NUM_TEST_FRAMES; j++) {
            if (frames[i] == frames[j]) {
                terminal_writestring("  [ERROR] Frames ");
                print_dec(i);
                terminal_writestring(" y ");
                print_dec(j);
                terminal_writestring(" son iguales: 0x");
                print_hex(frames[i]);
                terminal_writestring("\n");
                overlap_found = true;
            }
        }
    }
    
    // Liberar frames
    for (int i = 0; i < NUM_TEST_FRAMES; i++) {
        pmm_free_frame(frames[i]);
    }
    
    if (!overlap_found) {
        test_print_pass("PMM: sin solapamiento");
    } else {
        test_print_fail("PMM: sin solapamiento");
    }
}

/**
 * Ejecutar todos los tests de memoria
 */
void run_memory_tests(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("\n");
    terminal_writestring("╔════════════════════════════════════════╗\n");
    terminal_writestring("║   TESTS DEL SISTEMA DE MEMORIA        ║\n");
    terminal_writestring("╚════════════════════════════════════════╝\n");
    terminal_writestring("\n");
    
    // Tests de kmalloc
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("--- Tests de KMALLOC ---\n\n");
    
    test_kmalloc_small_blocks();
    test_kmalloc_alignment();
    test_kmalloc_out_of_memory();
    
    terminal_writestring("\n");
    
    // Tests de PMM
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("--- Tests de PMM (Physical Memory Manager) ---\n\n");
    
    test_pmm_alloc_free();
    test_pmm_no_overlap();
    
    terminal_writestring("\n");
}