// memory.c - Gestión avanzada de memoria física (PMM mejorado)
#include "memory.h"
#include "terminal.h"
#include "string.h"
#include "multiboot_header.h"

// Información global de memoria
static memory_info_t memory_info;

// PMM - Bitmap allocator mejorado
static uint32_t* pmm_bitmap = 0;
static uint64_t pmm_bitmap_size = 0;
static uint64_t pmm_total_frames = 0;
static uint64_t pmm_used_frames = 0;
static uint64_t pmm_reserved_frames = 0;
static uint64_t pmm_memory_base = 0;
static uint64_t pmm_memory_size = 0;

// ========== FUNCIONES AUXILIARES DEL BITMAP ==========

/**
 * Verificar si un frame está usado
 */
static inline bool pmm_test_frame(uint64_t frame_index) {
    if (frame_index >= pmm_total_frames) return true;
    uint64_t index = frame_index / 32;
    uint32_t bit = frame_index % 32;
    return (pmm_bitmap[index] & (1 << bit)) != 0;
}

/**
 * Marcar un frame como usado
 */
static inline void pmm_set_frame(uint64_t frame_index) {
    if (frame_index >= pmm_total_frames) return;
    uint64_t index = frame_index / 32;
    uint32_t bit = frame_index % 32;
    pmm_bitmap[index] |= (1 << bit);
}

/**
 * Marcar un frame como libre
 */
static inline void pmm_clear_frame(uint64_t frame_index) {
    if (frame_index >= pmm_total_frames) return;
    uint64_t index = frame_index / 32;
    uint32_t bit = frame_index % 32;
    pmm_bitmap[index] &= ~(1 << bit);
}

/**
 * Buscar bloques contiguos de frames libres (optimizado)
 * Devuelve el índice del primer frame del bloque, o -1 si no se encuentra
 */
static int64_t pmm_find_contiguous_frames(size_t count) {
    if (count == 0) return -1;
    if (count == 1) {
        // Optimización para el caso común de 1 frame
        for (uint64_t i = 0; i < pmm_total_frames; i++) {
            if (!pmm_test_frame(i)) {
                return (int64_t)i;
            }
        }
        return -1;
    }

    // Para múltiples frames, buscar bloques contiguos
    uint64_t contiguous = 0;
    uint64_t start = 0;

    for (uint64_t i = 0; i < pmm_total_frames; i++) {
        if (!pmm_test_frame(i)) {
            if (contiguous == 0) {
                start = i;
            }
            contiguous++;
            
            if (contiguous == count) {
                return (int64_t)start;
            }
        } else {
            contiguous = 0;
        }
    }

    return -1;
}

// ========== DETECCIÓN DE MEMORIA ==========

void memory_detect(uint32_t multiboot_info) {
    // Limpiar estructura
    memory_info.count = 0;
    memory_info.total_memory = 0;
    memory_info.available_memory = 0;
    
    if (multiboot_info == 0) {
        terminal_writestring("No hay información Multiboot2 disponible\n");
        return;
    }
    
    // Verificar alineación
    if (multiboot_info & 7) {
        terminal_writestring("Puntero Multiboot no alineado: 0x");
        print_hex(multiboot_info);
        terminal_writestring("\n");
        return;
    }
    
    uint32_t* mbi_ptr = (uint32_t*)multiboot_info;
    uint32_t total_size = mbi_ptr[0];
    
    terminal_writestring("Tamaño estructura Multiboot2: ");
    print_dec(total_size);
    terminal_writestring(" bytes\n");
    
    struct multiboot_tag* tag = (struct multiboot_tag*)(multiboot_info + 8);
    
    terminal_writestring("Analizando información Multiboot2...\n");
    
    while (tag->type != MULTIBOOT_TAG_TYPE_END && 
           (uint8_t*)tag < (uint8_t*)multiboot_info + total_size) {
        
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
                struct multiboot_tag_basic_meminfo* meminfo = 
                    (struct multiboot_tag_basic_meminfo*)tag;
                
                uint64_t low_memory = meminfo->mem_lower * 1024;
                uint64_t high_memory = meminfo->mem_upper * 1024;
                
                memory_info.total_memory = low_memory + high_memory;
                memory_info.available_memory = memory_info.total_memory;
                
                terminal_writestring("Memoria básica: ");
                print_dec(low_memory / 1024);
                terminal_writestring("KB baja + ");
                print_dec(high_memory / 1024);
                terminal_writestring("KB alta = ");
                print_dec(memory_info.total_memory / (1024 * 1024));
                terminal_writestring("MB total\n");
                break;
            }
            
            case MULTIBOOT_TAG_TYPE_MMAP: {
                struct multiboot_tag_mmap* mmap = (struct multiboot_tag_mmap*)tag;
                uint8_t* entry_ptr = (uint8_t*)mmap->entries;
                
                terminal_writestring("Mapa de memoria detectado:\n");
                
                while (entry_ptr < (uint8_t*)tag + tag->size) {
                    struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*)entry_ptr;
                    
                    if (memory_info.count < MEMORY_MAP_SIZE) {
                        memory_info.entries[memory_info.count].base_addr = entry->addr;
                        memory_info.entries[memory_info.count].length = entry->len;
                        memory_info.entries[memory_info.count].type = entry->type;
                        
                        memory_info.total_memory += entry->len;
                        
                        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                            memory_info.available_memory += entry->len;
                        }
                        
                        if (memory_info.count < 5) {
                            terminal_writestring("   ");
                            print_hex(entry->addr);
                            terminal_writestring(" - ");
                            print_hex(entry->addr + entry->len - 1);
                            terminal_writestring(" (");
                            print_dec(entry->len / 1024);
                            terminal_writestring(" KB) - ");
                            
                            switch (entry->type) {
                                case MULTIBOOT_MEMORY_AVAILABLE:
                                    terminal_writestring("DISPONIBLE");
                                    break;
                                case MULTIBOOT_MEMORY_RESERVED:
                                    terminal_writestring("RESERVADA");
                                    break;
                                case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                                    terminal_writestring("ACPI RECUPERABLE");
                                    break;
                                case MULTIBOOT_MEMORY_NVS:
                                    terminal_writestring("NVS");
                                    break;
                                case MULTIBOOT_MEMORY_BADRAM:
                                    terminal_writestring("BAD RAM");
                                    break;
                                default:
                                    terminal_writestring("DESCONOCIDA");
                                    break;
                            }
                            terminal_writestring("\n");
                        }
                        
                        memory_info.count++;
                    }
                    entry_ptr += mmap->entry_size;
                }
                
                if (memory_info.count >= 5) {
                    terminal_writestring("   ... y ");
                    print_dec(memory_info.count - 5);
                    terminal_writestring(" regiones más\n");
                }
                break;
            }
            
            case MULTIBOOT_TAG_TYPE_CMDLINE: {
                struct multiboot_tag_string* cmdline = (struct multiboot_tag_string*)tag;
                terminal_writestring("Línea de comandos: ");
                terminal_writestring(cmdline->string);
                terminal_writestring("\n");
                break;
            }
            
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
                struct multiboot_tag_string* bootloader = (struct multiboot_tag_string*)tag;
                terminal_writestring("Bootloader: ");
                terminal_writestring(bootloader->string);
                terminal_writestring("\n");
                break;
            }
        }
        
        tag = (struct multiboot_tag*)((uint8_t*)tag + ((tag->size + 7) & ~7));
    }
    
    terminal_writestring("Detección completada: ");
    print_dec(memory_info.count);
    terminal_writestring(" regiones, ");
    print_dec(memory_info.total_memory / (1024 * 1024));
    terminal_writestring("MB total, ");
    print_dec(memory_info.available_memory / (1024 * 1024));
    terminal_writestring("MB disponible\n");
}

memory_info_t* memory_get_info(void) {
    return &memory_info;
}

// ========== PHYSICAL MEMORY MANAGER (PMM) MEJORADO ==========

/**
 * Encontrar ubicación para el bitmap
 */
static uint64_t find_bitmap_location(uint64_t bitmap_size) {
    memory_info_t* mem_info = memory_get_info();
    
    for (size_t i = 0; i < mem_info->count; i++) {
        if (mem_info->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t base = mem_info->entries[i].base_addr;
            uint64_t length = mem_info->entries[i].length;
            
            if (base < 0x200000) {
                if (length > (0x200000 - base)) {
                    base = 0x200000;
                    length -= (0x200000 - mem_info->entries[i].base_addr);
                } else {
                    continue;
                }
            }
            
            if (length >= bitmap_size + (2 * PAGE_SIZE)) {
                return base + PAGE_SIZE;
            }
        }
    }
    
    return 0x200000;
}

void pmm_init(uint64_t memory_base, uint64_t memory_size) {
    pmm_memory_base = memory_base;
    pmm_memory_size = memory_size;
    
    // Calcular número total de frames
    pmm_total_frames = memory_size / PAGE_SIZE;
    pmm_used_frames = 0;
    pmm_reserved_frames = 0;
    
    // Calcular tamaño del bitmap
    pmm_bitmap_size = (pmm_total_frames + 31) / 32;
    uint64_t bitmap_bytes = pmm_bitmap_size * sizeof(uint32_t);
    
    // Encontrar ubicación para el bitmap
    pmm_bitmap = (uint32_t*)find_bitmap_location(bitmap_bytes);
    
    // Limpiar bitmap (toda la memoria disponible inicialmente)
    for (uint64_t i = 0; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0;
    }
    
    // Marcar frames como usados según el memory map
    memory_info_t* mem_info = memory_get_info();
    for (size_t i = 0; i < mem_info->count; i++) {
        if (mem_info->entries[i].type != MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t base = mem_info->entries[i].base_addr;
            uint64_t length = mem_info->entries[i].length;
            
            if (base >= pmm_memory_base && base < pmm_memory_base + pmm_memory_size) {
                uint64_t start_frame = (base - pmm_memory_base) / PAGE_SIZE;
                uint64_t end_frame = start_frame + (length + PAGE_SIZE - 1) / PAGE_SIZE;
                
                if (end_frame > pmm_total_frames) {
                    end_frame = pmm_total_frames;
                }
                
                for (uint64_t frame = start_frame; frame < end_frame; frame++) {
                    pmm_set_frame(frame);
                    pmm_used_frames++;
                }
            }
        }
    }
    
    // Marcar el área del bitmap como usada
    uint64_t bitmap_start_frame = ((uint64_t)pmm_bitmap - pmm_memory_base) / PAGE_SIZE;
    uint64_t bitmap_frames = (bitmap_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint64_t i = 0; i < bitmap_frames; i++) {
        uint64_t frame = bitmap_start_frame + i;
        if (frame < pmm_total_frames) {
            if (!pmm_test_frame(frame)) {
                pmm_set_frame(frame);
                pmm_used_frames++;
            }
        }
    }
    pmm_reserved_frames += bitmap_frames;
    
    // Marcar kernel como usado
    uint64_t kernel_frames = (0x100000 + 0x100000) / PAGE_SIZE;
    for (uint64_t i = 0; i < kernel_frames && i < pmm_total_frames; i++) {
        if (!pmm_test_frame(i)) {
            pmm_set_frame(i);
            pmm_used_frames++;
        }
    }
    pmm_reserved_frames += kernel_frames;
    
    terminal_writestring("[OK] PMM inicializado: ");
    print_dec(pmm_total_frames);
    terminal_writestring(" frames (");
    print_dec(pmm_total_frames * PAGE_SIZE / (1024 * 1024));
    terminal_writestring(" MB)\n");
}

// ========== ALLOCACIÓN Y LIBERACIÓN ==========

uint64_t pmm_alloc_frame(void) {
    int64_t frame_index = pmm_find_contiguous_frames(1);
    
    if (frame_index < 0) {
        return 0; // Sin memoria disponible
    }
    
    pmm_set_frame((uint64_t)frame_index);
    pmm_used_frames++;
    
    return pmm_memory_base + ((uint64_t)frame_index * PAGE_SIZE);
}

void pmm_free_frame(uint64_t frame_addr) {
    if (frame_addr < pmm_memory_base || 
        frame_addr >= pmm_memory_base + pmm_memory_size) {
        return; // Dirección inválida
    }
    
    uint64_t frame_index = (frame_addr - pmm_memory_base) / PAGE_SIZE;
    
    if (frame_index < pmm_total_frames && pmm_test_frame(frame_index)) {
        pmm_clear_frame(frame_index);
        if (pmm_used_frames > 0) {
            pmm_used_frames--;
        }
    }
}

// ========== ALLOCACIÓN Y LIBERACIÓN MÚLTIPLE (NUEVO) ==========

uint64_t pmm_alloc_frames(size_t count) {
    if (count == 0) return 0;
    
    int64_t start_frame = pmm_find_contiguous_frames(count);
    
    if (start_frame < 0) {
        return 0; // No hay suficientes frames contiguos
    }
    
    // Marcar todos los frames como usados
    for (size_t i = 0; i < count; i++) {
        pmm_set_frame((uint64_t)(start_frame + i));
    }
    
    pmm_used_frames += count;
    
    return pmm_memory_base + ((uint64_t)start_frame * PAGE_SIZE);
}

void pmm_free_frames(uint64_t start_addr, size_t count) {
    if (count == 0) return;
    
    if (start_addr < pmm_memory_base || 
        start_addr >= pmm_memory_base + pmm_memory_size) {
        return; // Dirección inválida
    }
    
    uint64_t start_frame = (start_addr - pmm_memory_base) / PAGE_SIZE;
    
    // Liberar todos los frames
    for (size_t i = 0; i < count; i++) {
        uint64_t frame_index = start_frame + i;
        
        if (frame_index < pmm_total_frames && pmm_test_frame(frame_index)) {
            pmm_clear_frame(frame_index);
            if (pmm_used_frames > 0) {
                pmm_used_frames--;
            }
        }
    }
}

// ========== ESTADÍSTICAS (NUEVO) ==========

void pmm_get_stats(pmm_stats_t* stats) {
    if (!stats) return;
    
    stats->total_frames = pmm_total_frames;
    stats->used_frames = pmm_used_frames;
    stats->free_frames = pmm_total_frames - pmm_used_frames;
    stats->reserved_frames = pmm_reserved_frames;
    
    stats->total_bytes = pmm_total_frames * PAGE_SIZE;
    stats->used_bytes = pmm_used_frames * PAGE_SIZE;
    stats->free_bytes = stats->free_frames * PAGE_SIZE;
    
    stats->usage_percent = pmm_total_frames > 0 
        ? (uint32_t)((pmm_used_frames * 100) / pmm_total_frames)
        : 0;
}

void pmm_print_stats(void) {
    pmm_stats_t stats;
    pmm_get_stats(&stats);
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("|=====================================|\n");
    terminal_writestring("  ESTADÍSTICAS DEL PMM (Detalladas) \n");
    terminal_writestring("|=====================================|\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Frames:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("  Total:     ");
    print_dec(stats.total_frames);
    terminal_writestring(" frames (");
    print_dec(stats.total_bytes / (1024 * 1024));
    terminal_writestring(" MB)\n");
    
    terminal_writestring("  Usados:    ");
    print_dec(stats.used_frames);
    terminal_writestring(" frames (");
    print_dec(stats.used_bytes / (1024 * 1024));
    terminal_writestring(" MB)\n");
    
    terminal_writestring("  Libres:    ");
    print_dec(stats.free_frames);
    terminal_writestring(" frames (");
    print_dec(stats.free_bytes / (1024 * 1024));
    terminal_writestring(" MB)\n");
    
    terminal_writestring("  Reservados: ");
    print_dec(stats.reserved_frames);
    terminal_writestring(" frames (kernel + bitmap)\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("\nUso de memoria:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("  Porcentaje: ");
    print_dec(stats.usage_percent);
    terminal_writestring("%\n");
    
    // Barra de progreso visual
    terminal_writestring("  [");
    uint32_t bar_length = 30;
    uint32_t filled = (stats.usage_percent * bar_length) / 100;

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
    terminal_writestring("\nInformación técnica:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("  Tamaño de página: ");
    print_dec(PAGE_SIZE);
    terminal_writestring(" bytes\n");
    terminal_writestring("  Bitmap en:        0x");
    print_hex((uint64_t)pmm_bitmap);
    terminal_writestring("\n");
    terminal_writestring("  Tamaño bitmap:    ");
    print_dec(pmm_bitmap_size * 4);
    terminal_writestring(" bytes\n");
}

// ========== FUNCIONES DE COMPATIBILIDAD ==========

uint64_t pmm_get_free_memory(void) {
    return (pmm_total_frames - pmm_used_frames) * PAGE_SIZE;
}

uint64_t pmm_get_total_memory(void) {
    return pmm_total_frames * PAGE_SIZE;
}

uint64_t pmm_get_used_memory(void) {
    return pmm_used_frames * PAGE_SIZE;
}

// ========== INFORMACIÓN DE MEMORIA ==========

void memory_print_info(void) {
    memory_info_t* mem_info = memory_get_info();
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("INFORMACIÓN DE MEMORIA\n");
    terminal_writestring("=========================\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Memoria total detectada: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    print_dec(mem_info->total_memory / (1024 * 1024));
    terminal_writestring(" MB\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Memoria disponible: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    print_dec(mem_info->available_memory / (1024 * 1024));
    terminal_writestring(" MB\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Regiones de memoria: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    print_dec(mem_info->count);
    terminal_writestring("\n");
    
    // Información del PMM si está inicializado
    if (pmm_bitmap != 0) {
        pmm_print_stats();
    }
}

void cmd_meminfo(int argc, char** argv) {
    (void)argc; (void)argv;
    memory_print_info();
}