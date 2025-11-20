; boot.asm - Bootloader Multiboot2 para x86_64 CORREGIDO
section .multiboot
align 8
multiboot_header_start:
    dd 0xe85250d6                ; magic number (multiboot2)
    dd 0                         ; architecture 0 (i386)
    dd multiboot_header_end - multiboot_header_start  ; header length
    dd -(0xe85250d6 + 0 + (multiboot_header_end - multiboot_header_start)) ; checksum

    ; End tag
    align 8
    dw 0                         ; type
    dw 0                         ; flags  
    dd 8                         ; size
multiboot_header_end:

section .bss
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table:
    resb 4096

; Stack separado para evitar problemas
stack_bottom:
    resb 16384  ; 16 KiB stack
stack_top:

; Variable global para almacenar el puntero Multiboot2 (32-bit!)
multiboot_info_ptr:
    resd 1                      ; Cambiado a 32-bit
multiboot_magic:
    resd 1                      ; Para guardar el magic number

section .text
bits 32
global start
extern kernel_main

start:
    ; El bootloader Multiboot2 nos pasa:
    ; EAX = Magic number (debe ser 0x36d76289)
    ; EBX = Puntero a estructura Multiboot2 (32-bit!)
    
    ; Guardar magic number y puntero inmediatamente
    mov [multiboot_magic], eax
    mov [multiboot_info_ptr], ebx
    
    ; Verificar magic number
    cmp eax, 0x36d76289
    jne .no_multiboot
    
    ; Configurar stack
    mov esp, stack_top
    
    call check_cpuid
    call check_long_mode
    call setup_page_tables
    call enable_paging
    
    lgdt [gdt64.pointer]
    jmp gdt64.code:long_mode_start

.no_multiboot:
    ; Mostrar error de magic number
    mov dword [0xb8000], 0x4f524f45  ; 'ER'
    mov dword [0xb8004], 0x4f3a4f52  ; 'R:'
    mov dword [0xb8008], 0x4f204f20  ; '  '
    mov byte  [0xb800a], '0'
    mov byte  [0xb800c], 'x'
    ; Mostrar el valor de EAX (magic number recibido)
    mov ecx, 8
    mov esi, 0xb800e
.magic_loop:
    rol eax, 4
    mov ebx, eax
    and ebx, 0x0F
    cmp ebx, 9
    jbe .digit
    add ebx, 7
.digit:
    add ebx, 0x30
    mov byte [esi], bl
    mov byte [esi+1], 0x4F
    add esi, 2
    loop .magic_loop
    hlt

check_cpuid:
    ; Intentar voltear el bit ID (21)
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    cmp eax, ecx
    je .no_cpuid
    ret
.no_cpuid:
    mov al, "1"
    jmp error

check_long_mode:
    ; Verificar si CPUID soporta extended functions
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    
    ; Verificar bit LM (long mode)
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    ret
.no_long_mode:
    mov al, "2"
    jmp error

setup_page_tables:
    ; Mapear primer entrada de PML4 a PDPT
    mov eax, p3_table
    or eax, 0b11                 ; present + writable
    mov [p4_table], eax
    
    ; Mapear entrada 256 de PML4 a PDPT (para higher half)
    mov eax, p3_table
    or eax, 0b11
    mov [p4_table + 256 * 8], eax
    
    ; Mapear primer entrada de PDPT a PD
    mov eax, p2_table
    or eax, 0b11
    mov [p3_table], eax
    
    ; Mapear misma entrada en PDPT higher half
    mov [p3_table + 256 * 8], eax
    
    ; Mapear 512 entradas de 2MB en el page directory
    mov ecx, 0
.map_p2_table:
    mov eax, 0x200000           ; 2MB
    mul ecx
    or eax, 0b10000011          ; present + writable + huge page
    mov [p2_table + ecx * 8], eax
    
    inc ecx
    cmp ecx, 512
    jne .map_p2_table
    
    ret

enable_paging:
    ; Cargar PML4 a CR3
    mov eax, p4_table
    mov cr3, eax
    
    ; Habilitar PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    
    ; Habilitar Long Mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    
    ; Habilitar paginación
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    
    ret

error:
    ; Mostrar código de error
    mov dword [0xb8000], 0x4f524f45  ; 'ER'
    mov dword [0xb8004], 0x4f3a4f52  ; 'R:'
    mov dword [0xb8008], 0x4f204f20  ; '  '
    mov byte  [0xb800a], al
    hlt

section .rodata
gdt64:
    dq 0                         ; entrada nula
.code: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; código 64-bit
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

section .text
bits 64
long_mode_start:
    ; Limpiar segmentos
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Configurar stack nuevamente en modo 64-bit
    mov rsp, stack_top
    
    ; Cargar parámetros para kernel_main
    ; En modo 64-bit, extendemos los valores 32-bit a 64-bit
    mov edi, [multiboot_magic]    ; Primer parámetro: magic number
    mov esi, [multiboot_info_ptr] ; Segundo parámetro: puntero a info
    
    ; Verificar que el puntero no sea NULL y esté alineado
    test esi, esi
    jz .no_multiboot_info
    
    test esi, 7                   ; Verificar alineación a 8 bytes
    jnz .unaligned_mbi
    
    ; Llamar al kernel en C con dos parámetros
    extern kernel_main
    call kernel_main
    
    ; En caso de retorno (no debería pasar)
    hlt
    jmp $

.no_multiboot_info:
    ; Mostrar error de puntero NULL
    mov rax, 0x4f524f454f4d4f4e  ; 'NO_MB_INFO'
    mov [0xb8000], rax
    hlt
    jmp $

.unaligned_mbi:
    mov rax, 0x4f4c4f414e554f20  ; 'UNALIGNED'
    mov [0xb8000], rax
    hlt
    jmp $