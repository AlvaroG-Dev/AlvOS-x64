// memory.h - Gestión básica de memoria
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

// Physical Memory Manager (PMM)
void pmm_init(uint64_t memory_base, uint64_t memory_size);
uint64_t pmm_alloc_frame(void);
void pmm_free_frame(uint64_t frame);
uint64_t pmm_get_free_memory(void);
uint64_t pmm_get_total_memory(void);
uint64_t pmm_get_used_memory(void);

// Detección de memoria
void memory_detect(uint64_t multiboot_info);
void memory_print_info(void);
memory_info_t* memory_get_info(void);

// Funciones del kernel para el shell
void cmd_meminfo(int argc, char** argv);

#endif