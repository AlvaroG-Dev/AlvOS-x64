; interrupts.asm - Interrupt Service Routines para x86_64
section .text
bits 64

; Macro para ISRs sin código de error
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push qword 0          ; Dummy error code
    push qword %1         ; Interrupt number
    jmp isr_common_stub
%endmacro

; Macro para ISRs con código de error
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push qword %1         ; Interrupt number
    jmp isr_common_stub
%endmacro

; Macro para IRQs
%macro IRQ 2
global irq%1
irq%1:
    push qword 0          ; Dummy error code
    push qword %2         ; IRQ number (remapped)
    jmp irq_common_stub
%endmacro

; Excepciones del CPU (ISR 0-31)
ISR_NOERRCODE 0     ; Division By Zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non Maskable Interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Into Detected Overflow
ISR_NOERRCODE 5     ; Out of Bounds
ISR_NOERRCODE 6     ; Invalid Opcode
ISR_NOERRCODE 7     ; No Coprocessor
ISR_ERRCODE   8     ; Double Fault
ISR_NOERRCODE 9     ; Coprocessor Segment Overrun
ISR_ERRCODE   10    ; Bad TSS
ISR_ERRCODE   11    ; Segment Not Present
ISR_ERRCODE   12    ; Stack Fault
ISR_ERRCODE   13    ; General Protection Fault
ISR_ERRCODE   14    ; Page Fault
ISR_NOERRCODE 15    ; Unknown Interrupt
ISR_NOERRCODE 16    ; Coprocessor Fault
ISR_ERRCODE   17    ; Alignment Check
ISR_NOERRCODE 18    ; Machine Check
ISR_NOERRCODE 19    ; Reserved
ISR_NOERRCODE 20    ; Reserved
ISR_NOERRCODE 21    ; Reserved
ISR_NOERRCODE 22    ; Reserved
ISR_NOERRCODE 23    ; Reserved
ISR_NOERRCODE 24    ; Reserved
ISR_NOERRCODE 25    ; Reserved
ISR_NOERRCODE 26    ; Reserved
ISR_NOERRCODE 27    ; Reserved
ISR_NOERRCODE 28    ; Reserved
ISR_NOERRCODE 29    ; Reserved
ISR_ERRCODE   30    ; Security Exception
ISR_NOERRCODE 31    ; Reserved

; IRQs (32-47) - Hardware Interrupts
IRQ 0,  32    ; PIT Timer
IRQ 1,  33    ; Keyboard
IRQ 2,  34    ; Cascade
IRQ 3,  35    ; COM2
IRQ 4,  36    ; COM1
IRQ 5,  37    ; LPT2
IRQ 6,  38    ; Floppy
IRQ 7,  39    ; LPT1
IRQ 8,  40    ; RTC
IRQ 9,  41    ; Free
IRQ 10, 42    ; Free
IRQ 11, 43    ; Free
IRQ 12, 44    ; PS2 Mouse
IRQ 13, 45    ; FPU
IRQ 14, 46    ; Primary ATA
IRQ 15, 47    ; Secondary ATA

extern isr_handler
extern irq_handler

; Stub común para ISRs
isr_common_stub:
    ; Guardar todos los registros
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; Llamar al handler en C
    mov rdi, rsp    ; Pasar el stack pointer como argumento
    call isr_handler
    
    ; Restaurar registros
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    
    ; Limpiar el número de interrupción y código de error
    add rsp, 16
    
    iretq

; Stub común para IRQs
irq_common_stub:
    ; Guardar todos los registros
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    ; Llamar al handler en C
    mov rdi, rsp    ; Pasar el stack pointer como argumento
    call irq_handler
    
    ; Restaurar registros
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    
    ; Limpiar el número de interrupción y código de error
    add rsp, 16
    
    iretq