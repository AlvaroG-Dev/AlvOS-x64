// constants.h - Constantes del sistema para eliminar magic numbers
#ifndef CONSTANTS_H
#define CONSTANTS_H

// Constantes de memoria
#define KERNEL_BASE_ADDRESS    0x100000    // 1MB
#define VGA_MEMORY_ADDRESS     0xB8000
#define STACK_SIZE             16384       // 16KB

// Constantes VGA
#define VGA_WIDTH              80
#define VGA_HEIGHT             25
#define VGA_BUFFER_SIZE        (VGA_WIDTH * VGA_HEIGHT * 2)

// Constantes del PIC
#define PIC1_COMMAND           0x20
#define PIC1_DATA              0x21
#define PIC2_COMMAND           0xA0
#define PIC2_DATA              0xA1
#define PIC_EOI                0x20

// Constantes del PIT
#define PIT_BASE_FREQUENCY     1193182     // Hz
#define PIT_COMMAND_PORT       0x43
#define PIT_CHANNEL0_PORT      0x40

// Constantes del teclado
#define KEYBOARD_DATA_PORT     0x60
#define KEYBOARD_BUFFER_SIZE   256

// Constantes Multiboot2
#define MULTIBOOT2_MAGIC       0x36d76289
#define MULTIBOOT2_HEADER_MAGIC 0xe85250d6

// Máscaras de bits del CPU
#define CR0_PAGING_BIT         (1 << 31)
#define CR4_PAE_BIT           (1 << 5)
#define EFER_LME_BIT          (1 << 8)

#endif