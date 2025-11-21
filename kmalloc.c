// kmalloc.c - Heap del kernel con soporte completo de allocación/liberación
#include "kmalloc.h"
#include "terminal.h"
#include "string.h"

// Estado global del heap
static heap_block_t* heap_head = NULL;
static bool heap_initialized = false;

// Estadísticas
static uint32_t total_allocations = 0;
static uint32_t total_frees = 0;
static uint32_t failed_allocations = 0;

// ========== FUNCIONES AUXILIARES ==========

/**
 * Alinear un valor al múltiplo superior más cercano
 */
static inline uint64_t align_up(uint64_t value, uint32_t alignment) {
    if (alignment == 0) return value;
    return (value + alignment - 1) & ~(alignment - 1);
}

/**
 * Verificar si un bloque es válido
 */
static inline bool is_valid_block(heap_block_t* block) {
    if (!block) return false;
    if (block->magic != HEAP_MAGIC) return false;
    if ((uint64_t)block < HEAP_START || (uint64_t)block >= HEAP_START + HEAP_SIZE) {
        return false;
    }
    return true;
}

/**
 * Obtener el puntero de datos de un bloque
 */
static inline void* block_to_ptr(heap_block_t* block) {
    return (void*)((uint8_t*)block + sizeof(heap_block_t));
}

/**
 * Obtener el bloque desde un puntero de datos
 */
static inline heap_block_t* ptr_to_block(void* ptr) {
    return (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
}

/**
 * Dividir un bloque grande en dos (split)
 * Se usa cuando un bloque libre es más grande de lo necesario
 */
static void split_block(heap_block_t* block, size_t size) {
    if (!block || !block->is_free) return;
    
    // Verificar que hay suficiente espacio para el split
    size_t min_block_size = sizeof(heap_block_t) + 16; // Mínimo 16 bytes de datos
    if (block->size < size + min_block_size) {
        return; // No vale la pena dividir
    }
    
    // Crear nuevo bloque en la parte sobrante
    heap_block_t* new_block = (heap_block_t*)((uint8_t*)block + sizeof(heap_block_t) + size);
    new_block->magic = HEAP_MAGIC;
    new_block->size = block->size - size - sizeof(heap_block_t);
    new_block->is_free = true;
    new_block->next = block->next;
    new_block->prev = block;
    
    // Actualizar bloques adyacentes
    if (block->next) {
        block->next->prev = new_block;
    }
    block->next = new_block;
    block->size = size;
}

/**
 * Fusionar bloques libres adyacentes (coalesce)
 * Evita fragmentación externa
 */
static void coalesce_blocks(heap_block_t* block) {
    if (!block || !block->is_free) return;
    
    // Fusionar con el siguiente bloque si es libre
    while (block->next && block->next->is_free) {
        heap_block_t* next = block->next;
        
        if (!is_valid_block(next)) break;
        
        // Expandir el bloque actual
        block->size += sizeof(heap_block_t) + next->size;
        block->next = next->next;
        
        if (next->next) {
            next->next->prev = block;
        }
        
        // Invalidar el bloque fusionado
        next->magic = 0;
    }
    
    // Fusionar con el bloque anterior si es libre
    if (block->prev && block->prev->is_free) {
        coalesce_blocks(block->prev);
    }
}

/**
 * Encontrar un bloque libre que se ajuste (first fit)
 */
static heap_block_t* find_free_block(size_t size) {
    heap_block_t* current = heap_head;
    
    while (current) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// ========== INICIALIZACIÓN ==========

void heap_init(void) {
    // Inicializar el primer bloque (todo el heap disponible)
    heap_head = (heap_block_t*)HEAP_START;
    heap_head->magic = HEAP_MAGIC;
    heap_head->size = HEAP_SIZE - sizeof(heap_block_t);
    heap_head->is_free = true;
    heap_head->next = NULL;
    heap_head->prev = NULL;
    
    // Limpiar estadísticas
    total_allocations = 0;
    total_frees = 0;
    failed_allocations = 0;
    
    heap_initialized = true;
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("[OK] Heap inicializado: 0x");
    print_hex(HEAP_START);
    terminal_writestring(" - 0x");
    print_hex(HEAP_START + HEAP_SIZE);
    terminal_writestring(" (");
    print_dec(HEAP_SIZE / 1024);
    terminal_writestring(" KB)\n");
}

// ========== ALLOCACIÓN ==========

void* kmalloc(size_t size) {
    return kmalloc_aligned(size, HEAP_ALIGNMENT);
}

void* kmalloc_aligned(size_t size, uint32_t alignment) {
    if (!heap_initialized) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("[ERROR] kmalloc: heap no inicializado\n");
        failed_allocations++;
        return NULL;
    }
    
    if (size == 0) {
        return NULL;
    }
    
    // Validar alineación
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        alignment = HEAP_ALIGNMENT;
    }
    
    // Alinear el tamaño
    size = align_up(size, alignment);
    
    // Buscar bloque libre
    heap_block_t* block = find_free_block(size);
    
    if (!block) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("[ERROR] kmalloc: sin memoria (");
        print_dec(size);
        terminal_writestring(" bytes)\n");
        failed_allocations++;
        return NULL;
    }
    
    // Dividir el bloque si es demasiado grande
    split_block(block, size);
    
    // Marcar como usado
    block->is_free = false;
    
    // Limpiar memoria allocada
    void* ptr = block_to_ptr(block);
    memset(ptr, 0, size);
    
    total_allocations++;
    
    return ptr;
}

void* kcalloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    
    // Verificar overflow
    if (nmemb != 0 && total_size / nmemb != size) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("[ERROR] kcalloc: overflow detectado\n");
        failed_allocations++;
        return NULL;
    }
    
    // kmalloc ya limpia la memoria, así que solo necesitamos allocar
    return kmalloc(total_size);
}

void* krealloc(void* ptr, size_t new_size) {
    if (!ptr) {
        return kmalloc(new_size);
    }
    
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    heap_block_t* block = ptr_to_block(ptr);
    
    if (!is_valid_block(block)) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("[ERROR] krealloc: bloque inválido\n");
        return NULL;
    }
    
    // Si el nuevo tamaño cabe en el bloque actual, no hacer nada
    if (new_size <= block->size) {
        return ptr;
    }
    
    // Allocar nuevo bloque
    void* new_ptr = kmalloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    // Copiar datos antiguos
    memcpy(new_ptr, ptr, block->size);
    
    // Liberar bloque antiguo
    kfree(ptr);
    
    return new_ptr;
}

// ========== LIBERACIÓN ==========

void kfree(void* ptr) {
    if (!ptr) {
        return; // free(NULL) es válido y no hace nada
    }
    
    if (!heap_initialized) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("[ERROR] kfree: heap no inicializado\n");
        return;
    }
    
    heap_block_t* block = ptr_to_block(ptr);
    
    if (!is_valid_block(block)) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("[ERROR] kfree: puntero inválido: 0x");
        print_hex((uint64_t)ptr);
        terminal_writestring("\n");
        return;
    }
    
    if (block->is_free) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
        terminal_writestring("[WARN] kfree: doble liberación detectada: 0x");
        print_hex((uint64_t)ptr);
        terminal_writestring("\n");
        return;
    }
    
    // Marcar como libre
    block->is_free = true;
    
    // Fusionar con bloques adyacentes libres
    coalesce_blocks(block);
    
    total_frees++;
}

// ========== ESTADÍSTICAS Y DEBUGGING ==========

void heap_get_stats(heap_stats_t* stats) {
    if (!stats || !heap_initialized) return;
    
    stats->heap_start = HEAP_START;
    stats->heap_end = HEAP_START + HEAP_SIZE;
    stats->total_size = HEAP_SIZE;
    stats->used_size = 0;
    stats->free_size = 0;
    stats->block_count = 0;
    stats->free_block_count = 0;
    stats->used_block_count = 0;
    stats->allocation_count = total_allocations;
    stats->free_count = total_frees;
    stats->failed_allocs = failed_allocations;
    
    heap_block_t* current = heap_head;
    while (current) {
        if (!is_valid_block(current)) break;
        
        stats->block_count++;
        
        if (current->is_free) {
            stats->free_size += current->size;
            stats->free_block_count++;
        } else {
            stats->used_size += current->size;
            stats->used_block_count++;
        }
        
        current = current->next;
    }
}

void heap_print_info(void) {
    if (!heap_initialized) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("Heap no inicializado.\n");
        return;
    }
    
    heap_stats_t stats;
    heap_get_stats(&stats);
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("\n|=====================================|\n");
    terminal_writestring("|  INFORMACIÓN DEL HEAP (Detallada)  |\n");
    terminal_writestring("|=====================================|\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Memoria:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("  Rango:      0x");
    print_hex(stats.heap_start);
    terminal_writestring(" - 0x");
    print_hex(stats.heap_end);
    terminal_writestring("\n");
    
    terminal_writestring("  Total:      ");
    print_dec(stats.total_size / 1024);
    terminal_writestring(" KB\n");
    
    terminal_writestring("  Usada:      ");
    print_dec(stats.used_size / 1024);
    terminal_writestring(" KB (");
    print_dec(stats.used_size);
    terminal_writestring(" bytes)\n");
    
    terminal_writestring("  Libre:      ");
    print_dec(stats.free_size / 1024);
    terminal_writestring(" KB (");
    print_dec(stats.free_size);
    terminal_writestring(" bytes)\n");
    
    uint64_t used_percent = stats.total_size > 0 
        ? (stats.used_size * 100) / stats.total_size
        : 0;
    terminal_writestring("  Uso:        ");
    print_dec(used_percent);
    terminal_writestring("%\n");
    
    // Barra de progreso
    terminal_writestring("  [");
    uint32_t bar_length = 30;
    uint32_t filled = (used_percent * bar_length) / 100;
    
    for (uint32_t i = 0; i < bar_length; i++) {
        if (i < filled) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
            terminal_putchar(0xDB); // Carácter lleno █
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK));
            terminal_putchar(SHADOW_CHAR); // Carácter sombreado ░
        }
    }
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("]\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("\nBloques:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("  Total:      ");
    print_dec(stats.block_count);
    terminal_writestring("\n");
    terminal_writestring("  Usados:     ");
    print_dec(stats.used_block_count);
    terminal_writestring("\n");
    terminal_writestring("  Libres:     ");
    print_dec(stats.free_block_count);
    terminal_writestring("\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("\nEstadísticas:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("  Allocaciones: ");
    print_dec(stats.allocation_count);
    terminal_writestring("\n");
    terminal_writestring("  Liberaciones: ");
    print_dec(stats.free_count);
    terminal_writestring("\n");
    terminal_writestring("  Fallidas:     ");
    print_dec(stats.failed_allocs);
    terminal_writestring("\n");
    
    // Fragmentación
    if (stats.free_block_count > 0) {
        uint64_t avg_free_size = stats.free_size / stats.free_block_count;
        terminal_writestring("  Tamaño medio libre: ");
        print_dec(avg_free_size);
        terminal_writestring(" bytes\n");
    }
}

void heap_print_blocks(void) {
    if (!heap_initialized) {
        terminal_writestring("Heap no inicializado.\n");
        return;
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("\nLISTA DE BLOQUES DEL HEAP:\n");
    terminal_writestring("==========================\n");
    
    heap_block_t* current = heap_head;
    uint32_t block_num = 0;
    
    while (current && block_num < 20) { // Limitar a 20 bloques para no saturar
        if (!is_valid_block(current)) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("[CORRUPTO]\n");
            break;
        }
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        terminal_writestring("Bloque ");
        print_dec(block_num);
        terminal_writestring(": 0x");
        print_hex((uint64_t)current);
        
        if (current->is_free) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
            terminal_writestring(" [LIBRE] ");
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring(" [USADO] ");
        }
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        print_dec(current->size);
        terminal_writestring(" bytes\n");
        
        current = current->next;
        block_num++;
    }
    
    if (current) {
        terminal_writestring("... (más bloques)\n");
    }
}

bool heap_validate(void) {
    if (!heap_initialized) return false;
    
    heap_block_t* current = heap_head;
    uint32_t blocks_checked = 0;
    
    while (current && blocks_checked < 10000) { // Límite de seguridad
        if (!is_valid_block(current)) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
            terminal_writestring("[ERROR] Heap corrupto en bloque: 0x");
            print_hex((uint64_t)current);
            terminal_writestring("\n");
            return false;
        }
        
        current = current->next;
        blocks_checked++;
    }
    
    return true;
}

void cmd_heapinfo(int argc, char** argv) {
    (void)argc;
    (void)argv;
    heap_print_info();
}