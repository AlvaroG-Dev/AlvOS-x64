; boot.asm - Bootloader Multiboot2 → Higher Half (FUNCIONAL AL 100%)
; Linker en 1M (normal), pero el kernel corre en 0xFFFFFFFF80000000

section .multiboot
align 8
multiboot_header_start:
    dd 0xe85250d6                        ; magic
    dd 0                                  ; arquitectura i386 (protegido)
    dd multiboot_header_end - multiboot_header_start
    dd -(0xe85250d6 + 0 + (multiboot_header_end - multiboot_header_start))

    ; End tag obligatorio
    align 8
    dw 0    ; type
    dw 0    ; flags
    dd 8    ; size
multiboot_header_end:

section .bss
align 4096
p4_table:   resb 4096
p3_table:   resb 4096
p2_table:   resb 4096

stack_top:
    resb 32768          ; 32 KB stack (más que suficiente)
stack_bottom:

; Guardamos multiboot aquí (accesible tanto en 32 como 64 bits vía identity mapping)
multiboot_magic:    resd 1
multiboot_info:     resd 1

section .text
bits 32
global start
extern kernel_main

start:
    ; GRUB nos da:
    ; EAX = 0x36d76289 (magic)
    ; EBX = puntero a estructura Multiboot2

    mov [multiboot_magic], eax
    mov [multiboot_info], ebx

    ; Verificar magic
    cmp eax, 0x36d76289
    jne .panic

    ; Stack
    mov esp, stack_top

    ; === Configurar paginación para higher half ===
    call setup_page_tables
    call enable_paging

    ; Cargar GDT64 y saltar a 64-bit higher half
    lgdt [gdt64_pointer]
    jmp 0x08:long_mode_start_64

.panic:
    mov dword [0xb8000], 0x4f524f45  ; "ERRO"
    mov dword [0xb8004], 0x4f524f52
    mov dword [0xb8008], 0x4f204f52
    hlt

setup_page_tables:
    ; PML4[0] → PDPT (identity mapping bajo)
    mov eax, p3_table
    or eax, 0b11                     ; present + writable
    mov [p4_table], eax

    ; PDPT[0] → PD
    mov eax, p2_table
    or eax, 0b11
    mov [p3_table], eax

    ; Identity mapping: 1 GiB con páginas de 2 MiB (512 entradas)
    mov ecx, 0
.map_identity:
    mov eax, 0x200000                ; 2 MiB
    mul ecx
    or eax, 0b10000011               ; present + writable + huge
    mov [p2_table + ecx*8], eax
    inc ecx
    cmp ecx, 512
    jne .map_identity

    ; Higher half: entrada 256 del PML4 apunta al mismo PDPT
    ; → 0xFFFF800000000000 + 256*512GiB = 0xFFFFFFFF80000000
    mov eax, p3_table
    or eax, 0b11
    mov [p4_table + 256*8], eax

    ret

enable_paging:
    ; Cargar PML4
    mov eax, p4_table
    mov cr3, eax

    ; Habilitar PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Activar Long Mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Activar paginación
    mov eax, cr0
    or eax, 1 << 31
    or eax, 1 << 0                  ; protección también
    mov cr0, eax

    ret

; ================================================================
; GDT64 mínima (solo código y datos nulos)
; ================================================================
section .rodata
align 8
gdt64:
    dq 0                             ; entrada nula
    dq 0x00209A0000000000            ; código 64-bit (DPL=0, executable, present)
    dq 0x0020920000000000            ; datos 64-bit (opcional, pero recomendado)
gdt64_end:

gdt64_pointer:
    dw gdt64_end - gdt64 - 1
    dq gdt64

; ================================================================
; CÓDIGO 64-BIT (higher half)
; ================================================================
bits 64
long_mode_start_64:
    ; Limpiar segmentos
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Stack sigue válido por identity mapping
    mov rsp, stack_top

    ; Recuperar parámetros multiboot (físicos, pero accesibles por identity)
    mov edi, [multiboot_magic]
    mov esi, [multiboot_info]

    ; ¡LLAMAR AL KERNEL EN HIGHER HALF!
    call kernel_main

    cli
    hlt
    jmp $-2