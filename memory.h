// memory.h - Gestión avanzada de memoria física (PMM mejorado)
#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Constantes de memoria
#define PAGE_SIZE 4096
#define KERNEL_BASE 0x100000  // 1MB
#define MEMORY_MAP_SIZE 256

// Estructuras para gestión de memoria
typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t acpi_attributes;
} memory_map_entry_t;

typedef struct {
    memory_map_entry_t entries[MEMORY_MAP_SIZE];
    size_t count;
    uint64_t total_memory;
    uint64_t available_memory;
} memory_info_t;

// Estadísticas del PMM
typedef struct {
    uint64_t total_frames;      // Total de frames disponibles
    uint64_t used_frames;       // Frames actualmente en uso
    uint64_t free_frames;       // Frames libres
    uint64_t reserved_frames;   // Frames reservados (kernel, bitmap, etc.)
    uint64_t total_bytes;       // Total en bytes
    uint64_t used_bytes;        // Bytes usados
    uint64_t free_bytes;        // Bytes libres
    uint32_t usage_percent;     // Porcentaje de uso (0-100)
} pmm_stats_t;

// ========== Physical Memory Manager (PMM) ==========

// Inicialización
void pmm_init(uint64_t memory_base, uint64_t memory_size);

// Allocación individual
uint64_t pmm_alloc_frame(void);
void pmm_free_frame(uint64_t frame);

// Allocación múltiple (NUEVO en Bloque 2.1)
uint64_t pmm_alloc_frames(size_t count);
void pmm_free_frames(uint64_t start_frame, size_t count);

// Estadísticas (NUEVO en Bloque 2.1)
void pmm_get_stats(pmm_stats_t* stats);
void pmm_print_stats(void);

// Información básica (mantenido por compatibilidad)
uint64_t pmm_get_free_memory(void);
uint64_t pmm_get_total_memory(void);
uint64_t pmm_get_used_memory(void);

// ========== Detección de memoria ==========
void memory_detect(uint32_t multiboot_info);
void memory_print_info(void);
memory_info_t* memory_get_info(void);

// ========== Comandos shell ==========
void cmd_meminfo(int argc, char** argv);

#endif // MEMORY_H