// kmalloc.h - Heap básico del kernel (allocator lineal simple)
#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Constantes del heap
#define HEAP_START      0x00300000    // 3MB (después del kernel y bitmap)
#define HEAP_SIZE       (1024 * 1024) // 1MB de heap inicial
#define HEAP_ALIGNMENT  4             // Alineación a 4 bytes

// Estructura de información del heap
typedef struct {
    uint64_t heap_start;
    uint64_t heap_end;
    uint64_t heap_current;
    uint64_t total_allocated;
    uint64_t allocation_count;
} heap_info_t;

// Funciones del heap
void heap_init(void);
void* kmalloc(size_t size);
void* kmalloc_aligned(size_t size, uint32_t alignment);
void heap_print_info(void);
heap_info_t* heap_get_info(void);

// Comando shell
void cmd_heapinfo(int argc, char** argv);

#endif