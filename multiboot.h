// multiboot.h - Estructuras para Multiboot2 (CORREGIDO)
#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

// Magic number para Multiboot2
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289

// Tipos de tags
#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE           1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT_TAG_TYPE_MODULE            3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT_TAG_TYPE_BOOTDEV           5
#define MULTIBOOT_TAG_TYPE_MMAP              6

// Tipos de memoria en MMAP
#define MULTIBOOT_MEMORY_AVAILABLE   1
#define MULTIBOOT_MEMORY_RESERVED    2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE  3
#define MULTIBOOT_MEMORY_NVS         4
#define MULTIBOOT_MEMORY_BADRAM      5

// Primero declarar multiboot_mmap_entry
struct multiboot_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
};

// Luego las otras estructuras que la usan
struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_string {
    uint32_t type;
    uint32_t size;
    char string[];
};

struct multiboot_tag_module {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    char cmdline[];
};

struct multiboot_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
};

struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_mmap_entry entries[];  
};

#endif