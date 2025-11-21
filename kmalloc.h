// kmalloc.h - Heap del kernel con soporte completo de allocación/liberación
#ifndef KMALLOC_H
#define KMALLOC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Constantes del heap
#define HEAP_START      0x00300000    // 3MB (después del kernel y bitmap)
#define HEAP_SIZE       (2 * 1024 * 1024) // 2MB de heap (aumentado)
#define HEAP_ALIGNMENT  16            // Alineación a 16 bytes (mejor para 64-bit)
#define HEAP_MAGIC      0xDEADBEEF    // Magic number para validación

// Estructura de bloque del heap (linked list)
typedef struct heap_block {
    uint32_t magic;              // Magic number para validación
    size_t size;                 // Tamaño del bloque (sin header)
    bool is_free;                // ¿Está libre?
    struct heap_block* next;     // Siguiente bloque
    struct heap_block* prev;     // Bloque anterior
} __attribute__((packed)) heap_block_t;

// Estructura de información del heap (estadísticas)
typedef struct {
    uint64_t heap_start;
    uint64_t heap_end;
    uint64_t total_size;
    uint64_t used_size;
    uint64_t free_size;
    uint32_t block_count;
    uint32_t free_block_count;
    uint32_t used_block_count;
    uint32_t allocation_count;   // Total de allocaciones hechas
    uint32_t free_count;         // Total de liberaciones hechas
    uint32_t failed_allocs;      // Allocaciones fallidas
} heap_stats_t;

// ========== Funciones del heap ==========

// Inicialización
void heap_init(void);

// Allocación y liberación
void* kmalloc(size_t size);
void* kmalloc_aligned(size_t size, uint32_t alignment);
void* kcalloc(size_t nmemb, size_t size);  // Allocar y limpiar
void* krealloc(void* ptr, size_t new_size); // Redimensionar
void kfree(void* ptr);

// Estadísticas y debugging
void heap_print_info(void);
void heap_get_stats(heap_stats_t* stats);
void heap_print_blocks(void);  // Imprimir todos los bloques (debug)
bool heap_validate(void);      // Validar integridad del heap

// Comandos shell
void cmd_heapinfo(int argc, char** argv);

#endif // KMALLOC_H