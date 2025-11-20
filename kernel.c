// kernel.c - Kernel principal con gestión de memoria real CORREGIDO
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "terminal.h"
#include "isr.h"
#include "timer.h"
#include "keyboard.h"
#include "string.h"
#include "panic.h"
#include "test_suite.h"
#include "shell.h"
#include "memory.h"
#include "multiboot_header.h"
#include "kmalloc.h"

// Estructura de la IDT
struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

#define IDT_ENTRIES 256
struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

// Configurar una entrada en la IDT
void idt_set_gate(uint8_t num, uint64_t handler, uint16_t selector, uint8_t flags) {
    idt[num].offset_low = handler & 0xFFFF;
    idt[num].selector = selector;
    idt[num].ist = 0;
    idt[num].type_attr = flags;
    idt[num].offset_mid = (handler >> 16) & 0xFFFF;
    idt[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[num].zero = 0;
}

extern void idt_load(uint64_t);

// Inicializar la IDT
void idt_install(void) {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (uint64_t)&idt;
    
    // Limpiar la IDT
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].ist = 0;
        idt[i].type_attr = 0;
        idt[i].offset_mid = 0;
        idt[i].offset_high = 0;
        idt[i].zero = 0;
    }
    
    // Cargar la IDT
    idt_load((uint64_t)&idtp);
}

// Función para debug de información Multiboot2
static void debug_multiboot_info(uint32_t magic, uint32_t multiboot_info) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    terminal_writestring("DEBUG MULTIBOOT2:\n");
    
    // Verificar magic number primero
    terminal_writestring("   Magic recibido: 0x");
    print_hex(magic);
    terminal_writestring(" (esperado: 0x");
    print_hex(MULTIBOOT2_BOOTLOADER_MAGIC);
    terminal_writestring(")\n");
    
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        terminal_writestring("Magic number incorrecto!\n");
        return;
    }
    
    if (multiboot_info == 0) {
        terminal_writestring("   multiboot_info es NULL\n");
        return;
    }
    
    // Verificar alineación (debe ser múltiplo de 8)
    if (multiboot_info & 7) {
        terminal_writestring("Puntero no alineado: 0x");
        print_hex(multiboot_info);
        terminal_writestring("\n");
        return;
    }
    
    // El puntero es de 32-bit, lo convertimos para uso en 64-bit
    uint32_t* mbi_ptr = (uint32_t*)multiboot_info;
    
    // Tamaño total
    uint32_t total_size = mbi_ptr[0];
    terminal_writestring("   Tamaño total: ");
    print_dec(total_size);
    terminal_writestring(" bytes\n");
    
    // Revisar tags (como en el ejemplo oficial)
    struct multiboot_tag* tag = (struct multiboot_tag*)(multiboot_info + 8);
    int tag_count = 0;
    
    terminal_writestring("   Tags encontrados:\n");
    
    while (tag->type != MULTIBOOT_TAG_TYPE_END && 
           (uint8_t*)tag < (uint8_t*)multiboot_info + total_size) {
        
        terminal_writestring("      Tag ");
        print_dec(tag_count);
        terminal_writestring(": tipo=0x");
        print_hex(tag->type);
        terminal_writestring(", tamaño=");
        print_dec(tag->size);
        terminal_writestring("\n");
        
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE: {
                struct multiboot_tag_string* cmdline = (struct multiboot_tag_string*)tag;
                terminal_writestring("         -> CMDLINE: ");
                terminal_writestring(cmdline->string);
                terminal_writestring("\n");
                break;
            }
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
                struct multiboot_tag_string* bootloader = (struct multiboot_tag_string*)tag;
                terminal_writestring("         -> BOOTLOADER: ");
                terminal_writestring(bootloader->string);
                terminal_writestring("\n");
                break;
            }
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO: {
                struct multiboot_tag_basic_meminfo* meminfo = 
                    (struct multiboot_tag_basic_meminfo*)tag;
                terminal_writestring("         -> BASIC_MEMINFO: ");
                print_dec(meminfo->mem_lower);
                terminal_writestring("KB baja, ");
                print_dec(meminfo->mem_upper);
                terminal_writestring("KB alta\n");
                break;
            }
            case MULTIBOOT_TAG_TYPE_MMAP:
                terminal_writestring("         -> MMAP (mapa de memoria)\n");
                break;
            default:
                terminal_writestring("         -> Tipo desconocido\n");
                break;
        }
        
        tag_count++;
        // Siguiente tag (alineado a 8 bytes) - COMO EN EL EJEMPLO OFICIAL
        tag = (struct multiboot_tag*)((uint8_t*)tag + ((tag->size + 7) & ~7));
    }
    
    terminal_writestring("   Total tags: ");
    print_dec(tag_count);
    terminal_writestring("\n");
}

// Función para encontrar la primera región de memoria disponible grande
static uint64_t find_available_memory_region(uint64_t* size) {
    memory_info_t* mem_info = memory_get_info();
    uint64_t largest_size = 0;
    uint64_t largest_base = 0;
    
    for (size_t i = 0; i < mem_info->count; i++) {
        if (mem_info->entries[i].type == MULTIBOOT_MEMORY_AVAILABLE) {
            if (mem_info->entries[i].length > largest_size) {
                largest_size = mem_info->entries[i].length;
                largest_base = mem_info->entries[i].base_addr;
            }
        }
    }
    
    if (size) *size = largest_size;
    return largest_base;
}

void kernel_main(uint32_t magic, uint32_t multiboot_info) {
    // Inicializar terminal
    terminal_initialize();
    
    // Banner
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("===================================\n");
    terminal_writestring("  Sistema Operativo 64-bit v0.4\n");
    terminal_writestring("    Multiboot2 Corregido\n");
    terminal_writestring("===================================\n\n");
    
    // Debuggear información Multiboot2 primero
    debug_multiboot_info(magic, multiboot_info);
    terminal_writestring("\n");
    
    // Verificar magic number (como en el ejemplo oficial)
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("Error: Magic number incorrecto: 0x");
        print_hex(magic);
        terminal_writestring("\n");
        return;
    }
    
    if (multiboot_info & 7) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("Error: Puntero Multiboot no alineado\n");
        return;
    }
    
    // Inicializar subsistemas básicos
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("[OK] Modo largo activado (64-bit)\n");
    terminal_writestring("[OK] Paginacion configurada\n");
    terminal_writestring("[OK] Terminal VGA inicializado\n");
    
    // Detectar memoria usando Multiboot2
    terminal_writestring("[..] Detectando memoria...\n");
    memory_detect(multiboot_info);
    
    // Encontrar región de memoria disponible para PMM
    uint64_t memory_size;
    uint64_t memory_base = find_available_memory_region(&memory_size);
    
    if (memory_base == 0 || memory_size < (2 * 1024 * 1024)) { // Mínimo 2MB
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("[ERR] No se encontró memoria disponible suficiente\n");
        // Usar fallback seguro
        memory_base = 0x100000;  // 1MB
        memory_size = 64 * 1024 * 1024; // 64MB
        terminal_writestring("[WARN] Usando memoria fallback: ");
        print_dec(memory_size / (1024 * 1024));
        terminal_writestring(" MB\n");
    } else {
        terminal_writestring("[OK] Memoria detectada: ");
        print_dec(memory_size / (1024 * 1024));
        terminal_writestring(" MB disponible\n");
    }
    
    // Inicializar PMM con memoria real
    pmm_init(memory_base, memory_size);
    heap_init();
    terminal_writestring("[OK] Heap del kernel inicializado\n");
    // Inicializar IDT
    idt_install();
    terminal_writestring("[OK] IDT inicializada\n");
    
    // Instalar ISRs e IRQs
    isr_install();
    terminal_writestring("[OK] ISRs instalados (excepciones)\n");
    terminal_writestring("[OK] IRQs instalados (hardware)\n");
    terminal_writestring("[OK] PIC remapeado\n");
    terminal_writestring("[OK] Interrupciones habilitadas\n");
    
    // Inicializar timer (100 Hz = 100 ticks por segundo)
    timer_install(100);
    terminal_writestring("[OK] Timer PIT configurado (100 Hz)\n");
    
    // Inicializar teclado
    keyboard_install();
    terminal_writestring("[OK] Teclado PS/2 inicializado\n");
    
    // Inicializar shell
    shell_init();
    
    // Mostrar información de memoria
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("\n💾 RESUMEN DE MEMORIA:\n");
    memory_print_info();
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("\n¿Ejecutar suite de pruebas? (s/n): ");
    
    // Esperar respuesta
    char response = keyboard_getchar();
    terminal_writestring("\n\n");
    
    if (response == 's' || response == 'S') {
        run_test_suite();
        terminal_writestring("\nTests completados. Iniciando shell...\n\n");
        shell_run();
    } else {
        shell_run();
    }
    
    // Loop principal
    while(1) {
        __asm__ volatile ("hlt");
    }
}