// kmalloc.c - Heap básico del kernel (allocator lineal simple)
#include "kmalloc.h"
#include "terminal.h"
#include "string.h"

// Estado global del heap
static heap_info_t heap_info;
static bool heap_initialized = false;

/**
 * Alinear un valor al múltiplo superior más cercano
 */
static inline uint64_t align_up(uint64_t value, uint32_t alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

/**
 * Inicializar el heap del kernel
 */
void heap_init(void) {
    heap_info.heap_start = HEAP_START;
    heap_info.heap_end = HEAP_START + HEAP_SIZE;
    heap_info.heap_current = HEAP_START;
    heap_info.total_allocated = 0;
    heap_info.allocation_count = 0;
    
    heap_initialized = true;
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("[OK] Heap inicializado: 0x");
    print_hex(heap_info.heap_start);
    terminal_writestring(" - 0x");
    print_hex(heap_info.heap_end);
    terminal_writestring(" (");
    print_dec(HEAP_SIZE / 1024);
    terminal_writestring(" KB)\n");
}

/**
 * Allocar memoria en el heap (allocator lineal simple)
 * NOTA: Por ahora sin kfree() - solo avanza el puntero
 * 
 * @param size Tamaño en bytes a allocar
 * @return Puntero a la memoria allocada, o NULL si falla
 */
void* kmalloc(size_t size) {
    return kmalloc_aligned(size, HEAP_ALIGNMENT);
}

/**
 * Allocar memoria con alineación específica
 * 
 * @param size Tamaño en bytes a allocar
 * @param alignment Alineación deseada (debe ser potencia de 2)
 * @return Puntero a la memoria allocada, o NULL si falla
 */
void* kmalloc_aligned(size_t size, uint32_t alignment) {
    if (!heap_initialized) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("[ERROR] kmalloc: heap no inicializado\n");
        return NULL;
    }
    
    if (size == 0) {
        return NULL;
    }
    
    // Validar que alignment es potencia de 2
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("[ERROR] kmalloc: alineación inválida\n");
        return NULL;
    }
    
    // Alinear la posición actual
    uint64_t aligned_current = align_up(heap_info.heap_current, alignment);
    
    // Verificar si hay espacio suficiente
    if (aligned_current + size > heap_info.heap_end) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("[ERROR] kmalloc: sin memoria (heap lleno)\n");
        terminal_writestring("  Solicitado: ");
        print_dec(size);
        terminal_writestring(" bytes\n");
        terminal_writestring("  Disponible: ");
        print_dec(heap_info.heap_end - aligned_current);
        terminal_writestring(" bytes\n");
        return NULL;
    }
    
    // Allocar memoria
    void* ptr = (void*)aligned_current;
    
    // Actualizar estado del heap
    heap_info.heap_current = aligned_current + size;
    heap_info.total_allocated += size;
    heap_info.allocation_count++;
    
    // Limpiar memoria allocada (opcional pero buena práctica)
    memset(ptr, 0, size);
    
    return ptr;
}

/**
 * Obtener información del heap
 */
heap_info_t* heap_get_info(void) {
    return &heap_info;
}

/**
 * Imprimir información detallada del heap
 */
void heap_print_info(void) {
    if (!heap_initialized) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("Heap no inicializado.\n");
        return;
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("INFORMACIÓN DEL HEAP\n");
    terminal_writestring("=========================\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Rango: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("0x");
    print_hex(heap_info.heap_start);
    terminal_writestring(" - 0x");
    print_hex(heap_info.heap_end);
    terminal_writestring("\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Tamaño total: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    print_dec((heap_info.heap_end - heap_info.heap_start) / 1024);
    terminal_writestring(" KB\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Usado: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    print_dec(heap_info.total_allocated / 1024);
    terminal_writestring(" KB (");
    print_dec(heap_info.total_allocated);
    terminal_writestring(" bytes)\n");
    
    uint64_t available = heap_info.heap_end - heap_info.heap_current;
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Disponible: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    print_dec(available / 1024);
    terminal_writestring(" KB (");
    print_dec(available);
    terminal_writestring(" bytes)\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Allocaciones: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    print_dec(heap_info.allocation_count);
    terminal_writestring("\n");
    
    // Calcular porcentaje de uso
    uint64_t used_percent = (heap_info.total_allocated * 100) / 
                            (heap_info.heap_end - heap_info.heap_start);
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Uso: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    print_dec(used_percent);
    terminal_writestring("%\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Próxima allocación: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("0x");
    print_hex(heap_info.heap_current);
    terminal_writestring("\n");
}

/**
 * Comando shell: heapinfo
 */
void cmd_heapinfo(int argc, char** argv) {
    (void)argc; (void)argv;
    heap_print_info();
}