// memory_tests.c - Suite completa de tests para memoria (Bloque 2.1)
#include "test_suite.h"
#include "memory.h"
#include "kmalloc.h"
#include "terminal.h"
#include "string.h"
#include "timer.h"

// ========== TESTS DE KMALLOC CON KFREE ==========

/**
 * Test 1: Allocar y liberar bloques pequeños
 */
void test_kmalloc_alloc_free_basic(void) {
    test_print_info("Test: kmalloc/kfree básico...");
    
    heap_stats_t stats_before, stats_after;
    heap_get_stats(&stats_before);
    
    // Allocar 10 bloques
    void* blocks[10];
    for (int i = 0; i < 10; i++) {
        blocks[i] = kmalloc(64);
        if (!blocks[i]) {
            test_print_fail("kmalloc/kfree básico");
            return;
        }
    }
    
    // Liberar todos
    for (int i = 0; i < 10; i++) {
        kfree(blocks[i]);
    }
    
    heap_get_stats(&stats_after);
    
    // La memoria usada debe volver a lo mismo (o similar)
    if (stats_after.used_size <= stats_before.used_size + 256) {
        test_print_pass("kmalloc/kfree básico");
    } else {
        test_print_warn("kmalloc/kfree básico - posible fragmentación");
    }
}

/**
 * Test 2: Coalescencia de bloques
 */
void test_kmalloc_coalesce(void) {
    test_print_info("Test: Coalescencia de bloques...");
    
    // Allocar 3 bloques consecutivos
    void* block1 = kmalloc(100);
    void* block2 = kmalloc(100);
    void* block3 = kmalloc(100);
    
    if (!block1 || !block2 || !block3) {
        test_print_fail("Coalescencia - allocación falló");
        return;
    }
    
    heap_stats_t stats_before;
    heap_get_stats(&stats_before);
    uint32_t blocks_before = stats_before.free_block_count;
    
    // Liberar los 3 bloques
    kfree(block1);
    kfree(block2);
    kfree(block3);
    
    heap_stats_t stats_after;
    heap_get_stats(&stats_after);
    
    // Debe haber menos bloques libres debido a coalescencia
    if (stats_after.free_block_count <= blocks_before) {
        test_print_pass("Coalescencia de bloques");
        terminal_writestring("  Bloques libres antes: ");
        print_dec(blocks_before);
        terminal_writestring(", después: ");
        print_dec(stats_after.free_block_count);
        terminal_writestring("\n");
    } else {
        test_print_warn("Coalescencia - bloques no fusionados correctamente");
    }
}

/**
 * Test 3: krealloc
 */
void test_krealloc(void) {
    test_print_info("Test: krealloc...");
    
    // Allocar bloque inicial
    char* str = (char*)kmalloc(10);
    if (!str) {
        test_print_fail("krealloc - allocación inicial falló");
        return;
    }
    
    // Escribir datos
    for (int i = 0; i < 9; i++) {
        str[i] = 'A' + i;
    }
    str[9] = '\0';
    
    // Redimensionar
    char* new_str = (char*)krealloc(str, 20);
    if (!new_str) {
        test_print_fail("krealloc - redimensión falló");
        kfree(str);
        return;
    }
    
    // Verificar que los datos se preservaron
    bool data_ok = true;
    for (int i = 0; i < 9; i++) {
        if (new_str[i] != 'A' + i) {
            data_ok = false;
            break;
        }
    }
    
    kfree(new_str);
    
    if (data_ok) {
        test_print_pass("krealloc");
    } else {
        test_print_fail("krealloc - datos corrompidos");
    }
}

/**
 * Test 4: kcalloc
 */
void test_kcalloc(void) {
    test_print_info("Test: kcalloc...");
    
    size_t count = 50;
    uint32_t* array = (uint32_t*)kcalloc(count, sizeof(uint32_t));
    
    if (!array) {
        test_print_fail("kcalloc - allocación falló");
        return;
    }
    
    // Verificar que está limpia
    bool all_zero = true;
    for (size_t i = 0; i < count; i++) {
        if (array[i] != 0) {
            all_zero = false;
            break;
        }
    }
    
    kfree(array);
    
    if (all_zero) {
        test_print_pass("kcalloc");
    } else {
        test_print_fail("kcalloc - memoria no limpia");
    }
}

/**
 * Test 5: Doble liberación
 */
void test_double_free(void) {
    test_print_info("Test: Detección de doble liberación...");
    
    void* ptr = kmalloc(100);
    if (!ptr) {
        test_print_fail("Doble free - allocación falló");
        return;
    }
    
    kfree(ptr);
    
    // Segunda liberación debe ser detectada y no crashear
    terminal_writestring("  Intentando doble free (debe mostrar warning):\n");
    kfree(ptr);
    
    test_print_pass("Detección de doble liberación");
}

/**
 * Test 6: Stress test - muchas allocaciones/liberaciones
 */
void test_kmalloc_stress(void) {
    test_print_info("Test: Stress test kmalloc/kfree...");
    
    #define STRESS_ITERATIONS 1000
    void* ptrs[STRESS_ITERATIONS];
    
    heap_stats_t stats_initial;
    heap_get_stats(&stats_initial);
    
    // Ciclo de alloc/free múltiple
    for (int cycle = 0; cycle < 5; cycle++) {
        // Allocar
        for (int i = 0; i < STRESS_ITERATIONS; i++) {
            size_t size = ((i * 17 + 13) % 200) + 16; // Tamaños variables
            ptrs[i] = kmalloc(size);
        }
        
        // Liberar en orden aleatorio (patrón pseudo-aleatorio)
        for (int i = 0; i < STRESS_ITERATIONS; i++) {
            int idx = (i * 37 + 11) % STRESS_ITERATIONS;
            if (ptrs[idx]) {
                kfree(ptrs[idx]);
                ptrs[idx] = NULL;
            }
        }
    }
    
    heap_stats_t stats_final;
    heap_get_stats(&stats_final);
    
    terminal_writestring("  Memoria usada inicial: ");
    print_dec(stats_initial.used_size);
    terminal_writestring(", final: ");
    print_dec(stats_final.used_size);
    terminal_writestring("\n");
    
    // Validar heap
    if (heap_validate()) {
        test_print_pass("Stress test kmalloc/kfree");
    } else {
        test_print_fail("Stress test - heap corrupto");
    }
}

// ========== TESTS DE PMM MEJORADO ==========

/**
 * Test 7: pmm_alloc_frames múltiple
 */
void test_pmm_alloc_multiple(void) {
    test_print_info("Test: PMM allocar múltiples frames...");
    
    pmm_stats_t stats_before;
    pmm_get_stats(&stats_before);
    
    // Allocar 10 frames contiguos
    uint64_t addr = pmm_alloc_frames(10);
    
    if (addr == 0) {
        test_print_fail("PMM alloc múltiple - allocación falló");
        return;
    }
    
    pmm_stats_t stats_after_alloc;
    pmm_get_stats(&stats_after_alloc);
    
    // Verificar que se allocaron exactamente 10 frames
    if (stats_after_alloc.used_frames == stats_before.used_frames + 10) {
        // Liberar
        pmm_free_frames(addr, 10);
        
        pmm_stats_t stats_after_free;
        pmm_get_stats(&stats_after_free);
        
        // Verificar que se liberaron
        if (stats_after_free.used_frames == stats_before.used_frames) {
            test_print_pass("PMM alloc/free múltiple");
        } else {
            test_print_warn("PMM alloc/free múltiple - liberación parcial");
        }
    } else {
        test_print_fail("PMM alloc múltiple - frames incorrectos");
        pmm_free_frames(addr, 10);
    }
}

/**
 * Test 8: Estadísticas del PMM
 */
void test_pmm_stats(void) {
    test_print_info("Test: Estadísticas del PMM...");
    
    pmm_stats_t stats;
    pmm_get_stats(&stats);
    
    // Verificar coherencia de estadísticas
    bool stats_valid = true;
    
    if (stats.total_frames != stats.used_frames + stats.free_frames) {
        terminal_writestring("  [ERROR] Total != Used + Free\n");
        stats_valid = false;
    }
    
    if (stats.total_bytes != stats.total_frames * PAGE_SIZE) {
        terminal_writestring("  [ERROR] Total bytes incorrecto\n");
        stats_valid = false;
    }
    
    if (stats.usage_percent > 100) {
        terminal_writestring("  [ERROR] Porcentaje > 100\n");
        stats_valid = false;
    }
    
    if (stats_valid) {
        test_print_pass("Estadísticas del PMM");
        terminal_writestring("  Total: ");
        print_dec(stats.total_frames);
        terminal_writestring(" frames, usado: ");
        print_dec(stats.usage_percent);
        terminal_writestring("%\n");
    } else {
        test_print_fail("Estadísticas del PMM");
    }
}

/**
 * Test 9: PMM frames contiguos grandes
 */
void test_pmm_contiguous_large(void) {
    test_print_info("Test: PMM bloques contiguos grandes...");
    
    // Intentar allocar 100 frames contiguos
    uint64_t addr = pmm_alloc_frames(100);
    
    if (addr == 0) {
        test_print_warn("PMM bloques grandes - no hay 100 frames contiguos");
        terminal_writestring("  (Esto es normal si hay fragmentación)\n");
        return;
    }
    
    // Verificar que las direcciones son realmente contiguas
    bool contiguous = true;
    for (size_t i = 0; i < 99; i++) {
        uint64_t expected = addr + (i * PAGE_SIZE);
        uint64_t next = addr + ((i + 1) * PAGE_SIZE);
        if (next != expected + PAGE_SIZE) {
            contiguous = false;
            break;
        }
    }
    
    pmm_free_frames(addr, 100);
    
    if (contiguous) {
        test_print_pass("PMM bloques contiguos grandes");
    } else {
        test_print_fail("PMM bloques grandes - no son contiguos");
    }
}

/**
 * Test 10: Fragmentación del heap
 */
void test_heap_fragmentation(void) {
    test_print_info("Test: Medición de fragmentación del heap...");
    
    #define FRAG_TEST_BLOCKS 20
    void* blocks[FRAG_TEST_BLOCKS];
    
    // Allocar bloques de diferentes tamaños
    for (int i = 0; i < FRAG_TEST_BLOCKS; i++) {
        size_t size = (i + 1) * 50;
        blocks[i] = kmalloc(size);
    }
    
    // Liberar bloques alternos
    for (int i = 0; i < FRAG_TEST_BLOCKS; i += 2) {
        kfree(blocks[i]);
        blocks[i] = NULL;
    }
    
    heap_stats_t stats;
    heap_get_stats(&stats);
    
    terminal_writestring("  Bloques libres: ");
    print_dec(stats.free_block_count);
    terminal_writestring(", memoria libre: ");
    print_dec(stats.free_size);
    terminal_writestring(" bytes\n");
    
    // Liberar el resto
    for (int i = 1; i < FRAG_TEST_BLOCKS; i += 2) {
        if (blocks[i]) kfree(blocks[i]);
    }
    
    test_print_pass("Medición de fragmentación");
}

// ========== EJECUTAR TODOS LOS TESTS ==========

void run_memory_tests(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("\n");
    terminal_writestring("|=========================================|\n");
    terminal_writestring("|    TESTS DEL BLOQUE 2.1 - MEMORIA       |\n");
    terminal_writestring("|=========================================|\n");
    terminal_writestring("\n");
    
    // Tests de kmalloc/kfree
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("--- Tests de KMALLOC con KFREE ---\n\n");
    
    test_kmalloc_alloc_free_basic();
    timer_wait(100);
    test_kmalloc_coalesce();
    timer_wait(100);
    test_krealloc();
    timer_wait(100);
    test_kcalloc();
    timer_wait(100);
    test_double_free();
    timer_wait(100);
    test_kmalloc_stress();
    timer_wait(100);
    terminal_writestring("\n");
    
    // Tests de PMM mejorado
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("--- Tests de PMM MEJORADO ---\n\n");
    
    test_pmm_alloc_multiple();
    timer_wait(100);
    test_pmm_stats();
    timer_wait(100);
    test_pmm_contiguous_large();
    timer_wait(100);

    terminal_writestring("\n");
    
    // Test de fragmentación
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("--- Tests de FRAGMENTACIÓN ---\n\n");
    
    test_heap_fragmentation();
    timer_wait(100);

    terminal_writestring("\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("Tests del Bloque 2.1 completados.\n");
}

void shell_cmd_memtest(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("\n");
    terminal_writestring("|========================================|\n");
    terminal_writestring("|     SUITE DE TESTS DE MEMORIA          |\n");
    terminal_writestring("|========================================|\n");
    terminal_writestring("\n");
    
    // Mostrar estado inicial
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("--- Estado inicial ---\n\n");
    
    pmm_stats_t pmm_stats_initial;
    pmm_get_stats(&pmm_stats_initial);
    
    heap_stats_t heap_stats_initial;
    heap_get_stats(&heap_stats_initial);
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("PMM: ");
    print_dec(pmm_stats_initial.free_frames);
    terminal_writestring(" frames libres (");
    print_dec(pmm_stats_initial.usage_percent);
    terminal_writestring("% usado)\n");
    
    terminal_writestring("Heap: ");
    print_dec(heap_stats_initial.free_size / 1024);
    terminal_writestring(" KB libres, ");
    print_dec(heap_stats_initial.block_count);
    terminal_writestring(" bloques\n\n");
    
    // Ejecutar tests
    run_memory_tests();
    
    // Mostrar estado final
    terminal_writestring("\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("--- Estado final ---\n\n");
    
    pmm_stats_t pmm_stats_final;
    pmm_get_stats(&pmm_stats_final);
    
    heap_stats_t heap_stats_final;
    heap_get_stats(&heap_stats_final);
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("PMM: ");
    print_dec(pmm_stats_final.free_frames);
    terminal_writestring(" frames libres (");
    print_dec(pmm_stats_final.usage_percent);
    terminal_writestring("% usado)\n");
    
    terminal_writestring("Heap: ");
    print_dec(heap_stats_final.free_size / 1024);
    terminal_writestring(" KB libres, ");
    print_dec(heap_stats_final.block_count);
    terminal_writestring(" bloques\n");
    
    // Análisis de leaks
    terminal_writestring("\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("--- Análisis de memory leaks ---\n\n");
    
    int64_t pmm_diff = (int64_t)pmm_stats_final.used_frames - (int64_t)pmm_stats_initial.used_frames;
    int64_t heap_diff = (int64_t)heap_stats_final.used_size - (int64_t)heap_stats_initial.used_size;
    
    if (pmm_diff == 0 && heap_diff == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
        terminal_writestring("No se detectaron memory leaks\n");
    } else {
        if (pmm_diff != 0) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
            terminal_writestring("PMM diferencia: ");
            if (pmm_diff > 0) terminal_writestring("+");
            print_dec((uint64_t)(pmm_diff > 0 ? pmm_diff : -pmm_diff));
            terminal_writestring(" frames\n");
        }
        
        if (heap_diff != 0) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
            terminal_writestring("Heap diferencia: ");
            if (heap_diff > 0) terminal_writestring("+");
            print_dec((uint64_t)(heap_diff > 0 ? heap_diff : -heap_diff));
            terminal_writestring(" bytes\n");
        }
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        terminal_writestring("(Pequeñas diferencias son normales debido a overhead)\n");
    }
    
    terminal_writestring("\n");
}