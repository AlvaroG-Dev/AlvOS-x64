; idt.asm - Carga de la tabla de descriptores de interrupciones
section .text
bits 64

global idt_load
idt_load:
    lidt [rdi]
    ret