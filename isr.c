// isr.c - Interrupt Service Routines y handlers
#include <stdint.h>
#include "isr.h"
#include "terminal.h"
#include "panic.h"

// Estructura del stack frame cuando ocurre una interrupción
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) registers_t;

// Nombres de las excepciones
const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"
};

// Declaraciones externas de los handlers en assembly
extern void isr0();  extern void isr1();  extern void isr2();  extern void isr3();
extern void isr4();  extern void isr5();  extern void isr6();  extern void isr7();
extern void isr8();  extern void isr9();  extern void isr10(); extern void isr11();
extern void isr12(); extern void isr13(); extern void isr14(); extern void isr15();
extern void isr16(); extern void isr17(); extern void isr18(); extern void isr19();
extern void isr20(); extern void isr21(); extern void isr22(); extern void isr23();
extern void isr24(); extern void isr25(); extern void isr26(); extern void isr27();
extern void isr28(); extern void isr29(); extern void isr30(); extern void isr31();

extern void irq0();  extern void irq1();  extern void irq2();  extern void irq3();
extern void irq4();  extern void irq5();  extern void irq6();  extern void irq7();
extern void irq8();  extern void irq9();  extern void irq10(); extern void irq11();
extern void irq12(); extern void irq13(); extern void irq14(); extern void irq15();

// Port I/O functions
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Remapear el PIC para evitar conflictos con las excepciones
void pic_remap(void) {
    uint8_t a1, a2;
    
    // Guardar máscaras
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);
    
    // Iniciar la secuencia de inicialización (en modo cascada)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    
    // Vector offsets del PIC
    outb(PIC1_DATA, 0x20);  // IRQ 0-7 -> interrupciones 32-39
    outb(PIC2_DATA, 0x28);  // IRQ 8-15 -> interrupciones 40-47
    
    // Informar a los PICs sobre su configuración en cascada
    outb(PIC1_DATA, 4);     // PIC1 tiene slave en IRQ2
    outb(PIC2_DATA, 2);     // PIC2 es slave en IRQ1 del master
    
    // Modo 8086
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    
    // Restaurar máscaras guardadas
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

// Enviar EOI (End Of Interrupt) al PIC
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8)
        outb(PIC2_COMMAND, PIC_EOI);
    outb(PIC1_COMMAND, PIC_EOI);
}

// Habilitar una IRQ específica
void irq_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) | (1 << irq);
    outb(port, value);
}

// Deshabilitar una IRQ específica
void irq_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

// Instalar los ISRs en la IDT
void isr_install(void) {
    // Instalar las excepciones (ISR 0-31)
    idt_set_gate(0,  (uint64_t)isr0,  0x08, 0x8E);
    idt_set_gate(1,  (uint64_t)isr1,  0x08, 0x8E);
    idt_set_gate(2,  (uint64_t)isr2,  0x08, 0x8E);
    idt_set_gate(3,  (uint64_t)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (uint64_t)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (uint64_t)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (uint64_t)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (uint64_t)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (uint64_t)isr8,  0x08, 0x8E);
    idt_set_gate(9,  (uint64_t)isr9,  0x08, 0x8E);
    idt_set_gate(10, (uint64_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint64_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint64_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint64_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint64_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint64_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint64_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint64_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint64_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint64_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint64_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint64_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint64_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint64_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint64_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint64_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint64_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint64_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint64_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint64_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint64_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint64_t)isr31, 0x08, 0x8E);
    
    // Remapear e instalar IRQs (32-47)
    pic_remap();
    
    idt_set_gate(32, (uint64_t)irq0,  0x08, 0x8E);
    idt_set_gate(33, (uint64_t)irq1,  0x08, 0x8E);
    idt_set_gate(34, (uint64_t)irq2,  0x08, 0x8E);
    idt_set_gate(35, (uint64_t)irq3,  0x08, 0x8E);
    idt_set_gate(36, (uint64_t)irq4,  0x08, 0x8E);
    idt_set_gate(37, (uint64_t)irq5,  0x08, 0x8E);
    idt_set_gate(38, (uint64_t)irq6,  0x08, 0x8E);
    idt_set_gate(39, (uint64_t)irq7,  0x08, 0x8E);
    idt_set_gate(40, (uint64_t)irq8,  0x08, 0x8E);
    idt_set_gate(41, (uint64_t)irq9,  0x08, 0x8E);
    idt_set_gate(42, (uint64_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint64_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint64_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint64_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint64_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint64_t)irq15, 0x08, 0x8E);
    
    // Habilitar interrupciones
    __asm__ volatile ("sti");
}

// Handler de excepciones del CPU
void isr_handler(registers_t *regs) {
    __asm__ volatile ("cli");  // Deshabilitar interrupciones inmediatamente
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    terminal_writestring("\n\n!!! EXCEPCION DEL CPU !!!\n");
    terminal_writestring("===========================\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    terminal_writestring("Tipo: ");
    terminal_writestring(exception_messages[regs->int_no]);
    terminal_writestring(" (");
    print_dec(regs->int_no);
    terminal_writestring(")\n");
    
    // Información detallada del error
    terminal_writestring("Error code: 0x");
    print_hex(regs->err_code);
    terminal_writestring("\n");
    
    // Información de los registros
    terminal_writestring("\nRegistros:\n");
    terminal_writestring("RAX: 0x"); print_hex(regs->rax); terminal_writestring("  ");
    terminal_writestring("RBX: 0x"); print_hex(regs->rbx); terminal_writestring("  ");
    terminal_writestring("RCX: 0x"); print_hex(regs->rcx); terminal_writestring("\n");
    terminal_writestring("RDX: 0x"); print_hex(regs->rdx); terminal_writestring("  ");
    terminal_writestring("RSI: 0x"); print_hex(regs->rsi); terminal_writestring("  ");
    terminal_writestring("RDI: 0x"); print_hex(regs->rdi); terminal_writestring("\n");
    terminal_writestring("RBP: 0x"); print_hex(regs->rbp); terminal_writestring("  ");
    terminal_writestring("RSP: 0x"); print_hex(regs->rsp); terminal_writestring("\n");
    
    // Información de la instrucción
    terminal_writestring("\nInstruccion:\n");
    terminal_writestring("RIP: 0x"); print_hex(regs->rip); terminal_writestring("\n");
    terminal_writestring("CS: 0x"); print_hex(regs->cs); terminal_writestring("  ");
    terminal_writestring("RFLAGS: 0x"); print_hex(regs->rflags); terminal_writestring("\n");
    terminal_writestring("SS: 0x"); print_hex(regs->ss); terminal_writestring("\n");
    
    // Stack trace básico (primeros 4 frames)
    terminal_writestring("\nStack trace (primeros 4 frames):\n");
    uint64_t *rsp = (uint64_t*)regs->rsp;
    for (int i = 0; i < 4 && i < 16; i++) {
        terminal_writestring("  [");
        print_dec(i);
        terminal_writestring("] 0x");
        print_hex(rsp[i]);
        terminal_writestring("\n");
    }
    
    terminal_writestring("\nSistema detenido por excepcion irrecuperable.\n");
    
    // Halt permanente
    while(1) {
        __asm__ volatile ("hlt");
    }
}



// Handlers externos
extern void timer_handler(void);
extern void keyboard_handler(void);

// Handler de IRQs (interrupciones de hardware)
void irq_handler(registers_t *regs) {
    // Llamar al handler específico según el IRQ
    switch (regs->int_no) {
        case 32:  // IRQ0 - Timer
            timer_handler();
            break;
        case 33:  // IRQ1 - Keyboard
            keyboard_handler();
            break;
        // Agregar más IRQs aquí según sea necesario
    }
    
    // Enviar EOI al PIC
    pic_send_eoi(regs->int_no - 32);
}