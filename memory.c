// memory.c - Gestión de memoria real usando Multiboot2
#include "memory.h"
#include "terminal.h"
#include "string.h"
#include "multiboot.h"

// Información global de memoria
static memory_info_t memory_info;

// PMM - Bitmap allocator
static uint32_t* pmm_bitmap = 0;
static uint64_t pmm_bitmap_size = 0;
static uint64_t pmm_total_frames = 0;
static uint64_t pmm_used_frames = 0;
static uint64_t pmm_memory_base = 0;
static uint64_t pmm_memory_size = 0;

// ========== DETECCIÓN DE MEMORIA ==========

// Obtener información de memoria de Multiboot2
void memory_detect(uint64_t multiboot_info) {
    // Limpiar estructura
    memory_info.count = 0;
    memory_info.total_memory = 0;
    memory_info.available_memory = 0;
    
    if (multiboot_info == 0) {
        terminal_writestring("❌ No hay información Multiboot2 disponible\n");
        return;
    }
    
    // Verificar magic number
    uint32_t magic = *(uint32_t*)multiboot_info;
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        terminal_writestring("❌ Magic number de Multiboot2 incorrecto\n");
        return;
    }
    
    // El puntero de Multiboot2 apunta al inicio de la estructura
    uint32_t total_size = *(uint32_t*)multiboot_info;
    struct multiboot_tag* tag = (struct multiboot_tag*)(multiboot_info + 8);
    
    while (tag->type != MULTIBOOT_TAG_TYPE_END && 
           (uint8_t*)tag < (uint8_t*)multiboot_info + total_size) {
        
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
                struct multiboot_tag_basic_meminfo* meminfo = 
                    (struct multiboot_tag_basic_meminfo*)tag;
                
                // Memoria baja (0-640KB) y alta (1MB+)
                uint64_t low_memory = meminfo->mem_lower * 1024;  // Convertir a bytes
                uint64_t high_memory = meminfo->mem_upper * 1024; // Convertir a bytes
                
                memory_info.total_memory = low_memory + high_memory;
                memory_info.available_memory = memory_info.total_memory;
                
                terminal_writestring("📊 Memoria básica: ");
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
                struct multiboot_mmap_entry* entry = mmap->entries;
                
                terminal_writestring("🗺️  Mapa de memoria detectado:\n");
                
                while ((uint8_t*)entry < (uint8_t*)tag + tag->size) {
                    if (memory_info.count < MEMORY_MAP_SIZE) {
                        memory_info.entries[memory_info.count].base_addr = entry->addr;
                        memory_info.entries[memory_info.count].length = entry->len;
                        memory_info.entries[memory_info.count].type = entry->type;
                        
                        // Sumar a memoria total
                        memory_info.total_memory += entry->len;
                        
                        // Sumar a memoria disponible si es de tipo disponible
                        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                            memory_info.available_memory += entry->len;
                        }
                        
                        // Mostrar entrada (solo las primeras 5 para no saturar)
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
                    entry = (struct multiboot_mmap_entry*)
                        ((uint8_t*)entry + mmap->entry_size);
                }
                
                if (memory_info.count >= 5) {
                    terminal_writestring("   ... y ");
                    print_dec(memory_info.count - 5);
                    terminal_writestring(" regiones más\n");
                }
                break;
            }
        }
        
        // Siguiente tag (alineado a 8 bytes)
        tag = (struct multiboot_tag*)((uint8_t*)tag + ((tag->size + 7) & ~7));
    }
    
    terminal_writestring("✅ Detección completada: ");
    print_dec(memory_info.count);
    terminal_writestring(" regiones, ");
    print_dec(memory_info.total_memory / (1024 * 1024));
    terminal_writestring("MB total, ");
    print_dec(memory_info.available_memory / (1024 * 1024));
    terminal_writestring("MB disponible\n");
}

// Obtener información de memoria
memory_info_t* memory_get_info(void) {
    return &memory_info;
}

// ========== PHYSICAL MEMORY MANAGER (PMM) ==========

// Encontrar una región de memoria disponible para el bitmap
static uint64_t find_bitmap_location(uint64_t bitmap_size) {
    memory_info_t* mem_info = memory_get_info();
    
    for (size_t i = 0; i < mem_info->count; i++) {
        if (mem_info->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t base = mem_info->entries[i].base_addr;
            uint64_t length = mem_info->entries[i].length;
            
            // Buscar después de 2MB para evitar área del kernel
            if (base < 0x200000) {
                base = 0x200000;
                length -= (0x200000 - mem_info->entries[i].base_addr);
            }
            
            // Verificar si hay espacio suficiente
            if (length >= bitmap_size + (2 * PAGE_SIZE)) {
                return base + PAGE_SIZE; // Dejar un page de margen
            }
        }
    }
    
    // Fallback: usar dirección fija después del kernel
    return 0x200000;
}

// Inicializar PMM con memoria real
void pmm_init(uint64_t memory_base, uint64_t memory_size) {
    pmm_memory_base = memory_base;
    pmm_memory_size = memory_size;
    
    // Calcular número total de frames (4KB cada uno)
    pmm_total_frames = memory_size / PAGE_SIZE;
    pmm_used_frames = 0;
    
    // Calcular tamaño del bitmap (1 bit por frame)
    pmm_bitmap_size = (pmm_total_frames + 31) / 32; // En términos de uint32_t
    uint64_t bitmap_bytes = pmm_bitmap_size * sizeof(uint32_t);
    
    // Encontrar ubicación para el bitmap
    pmm_bitmap = (uint32_t*)find_bitmap_location(bitmap_bytes);
    
    // Limpiar bitmap (toda la memoria disponible inicialmente)
    for (uint64_t i = 0; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0;
    }
    
    // Marcar frames como usados según el memory map real
    memory_info_t* mem_info = memory_get_info();
    for (size_t i = 0; i < mem_info->count; i++) {
        if (mem_info->entries[i].type != MULTIBOOT_MEMORY_AVAILABLE) {
            // Esta región no está disponible, marcar sus frames como usados
            uint64_t base = mem_info->entries[i].base_addr;
            uint64_t length = mem_info->entries[i].length;
            
            // Solo procesar si está dentro de nuestra área gestionada
            if (base >= pmm_memory_base && base < pmm_memory_base + pmm_memory_size) {
                uint64_t start_frame = (base - pmm_memory_base) / PAGE_SIZE;
                uint64_t end_frame = start_frame + (length + PAGE_SIZE - 1) / PAGE_SIZE;
                
                if (end_frame > pmm_total_frames) {
                    end_frame = pmm_total_frames;
                }
                
                for (uint64_t frame = start_frame; frame < end_frame; frame++) {
                    pmm_bitmap[frame / 32] |= (1 << (frame % 32));
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
            pmm_bitmap[frame / 32] |= (1 << (frame % 32));
            pmm_used_frames++;
        }
    }
    
    terminal_writestring("[OK] PMM: ");
    print_dec(pmm_total_frames);
    terminal_writestring(" frames (");
    print_dec(pmm_total_frames * PAGE_SIZE / (1024 * 1024));
    terminal_writestring(" MB), bitmap en 0x");
    print_hex((uint64_t)pmm_bitmap);
    terminal_writestring("\n");
}

void memory_print_info(void) {
    memory_info_t* mem_info = memory_get_info();
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("💾 INFORMACIÓN DE MEMORIA\n");
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
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("\n🧮 GESTOR DE MEMORIA FÍSICA (PMM)\n");
        terminal_writestring("=================================\n");
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_writestring("Área gestionada: 0x");
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        print_hex(pmm_memory_base);
        terminal_writestring(" - 0x");
        print_hex(pmm_memory_base + pmm_memory_size - 1);
        terminal_writestring("\n");
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_writestring("Frames totales: ");
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        print_dec(pmm_total_frames);
        terminal_writestring(" (");
        print_dec(pmm_total_frames * PAGE_SIZE / 1024);
        terminal_writestring(" KB)\n");
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_writestring("Frames usados: ");
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        print_dec(pmm_used_frames);
        terminal_writestring(" (");
        print_dec(pmm_used_frames * PAGE_SIZE / 1024);
        terminal_writestring(" KB)\n");
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_writestring("Frames libres: ");
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        print_dec(pmm_total_frames - pmm_used_frames);
        terminal_writestring(" (");
        print_dec((pmm_total_frames - pmm_used_frames) * PAGE_SIZE / 1024);
        terminal_writestring(" KB)\n");
        
        // Calcular porcentaje de uso
        uint64_t used_percent = (pmm_used_frames * 100) / pmm_total_frames;
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_writestring("Uso: ");
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        print_dec(used_percent);
        terminal_writestring("%\n");
    }
}


// Reservar un frame
uint64_t pmm_alloc_frame(void) {
    for (uint64_t i = 0; i < pmm_total_frames; i++) {
        uint64_t index = i / 32;
        uint32_t bit = i % 32;
        
        if (!(pmm_bitmap[index] & (1 << bit))) {
            // Frame libre encontrado, marcarlo como usado
            pmm_bitmap[index] |= (1 << bit);
            pmm_used_frames++;
            
            // Devolver dirección física del frame
            return pmm_memory_base + (i * PAGE_SIZE);
        }
    }
    
    // No hay frames libres
    return 0;
}

// Liberar un frame
void pmm_free_frame(uint64_t frame) {
    // Calcular índice del frame
    uint64_t frame_index = (frame - pmm_memory_base) / PAGE_SIZE;
    
    if (frame_index < pmm_total_frames) {
        uint64_t index = frame_index / 32;
        uint32_t bit = frame_index % 32;
        
        // Verificar que estaba realmente usado
        if (pmm_bitmap[index] & (1 << bit)) {
            pmm_bitmap[index] &= ~(1 << bit);
            pmm_used_frames--;
        }
    }
}

// Obtener memoria libre
uint64_t pmm_get_free_memory(void) {
    return (pmm_total_frames - pmm_used_frames) * PAGE_SIZE;
}

// Obtener memoria total
uint64_t pmm_get_total_memory(void) {
    return pmm_total_frames * PAGE_SIZE;
}

// Obtener memoria usada
uint64_t pmm_get_used_memory(void) {
    return pmm_used_frames * PAGE_SIZE;
}

// ========== COMANDO SHELL ==========

// Comando: meminfo
void cmd_meminfo(int argc, char** argv) {
    (void)argc; (void)argv;
    memory_print_info();
}