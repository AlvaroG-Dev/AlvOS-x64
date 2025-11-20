# 🏗️ Arquitectura del Sistema - AlvOS

Documentación técnica detallada de la arquitectura interna del sistema operativo.

---

## 📐 Diagrama de Módulos

```
┌─────────────────────────────────────────────────────────────┐
│                        APLICACIONES                          │
│                    (Shell, Test Suite)                       │
└────────────────────┬────────────────────────────────────────┘
                     │
┌────────────────────┴────────────────────────────────────────┐
│                    KERNEL (Ring 0)                           │
│ ┌──────────────────────────────────────────────────────┐    │
│ │              SUBSISTEMA DE MEMORIA                   │    │
│ │  ┌────────────┐  ┌────────────┐  ┌────────────┐     │    │
│ │  │    PMM     │  │   Heap     │  │  Paging    │     │    │
│ │  │  (Bitmap)  │  │ (kmalloc)  │  │ (Identity) │     │    │
│ │  └────────────┘  └────────────┘  └────────────┘     │    │
│ └──────────────────────────────────────────────────────┘    │
│                                                              │
│ ┌──────────────────────────────────────────────────────┐    │
│ │              SUBSISTEMA DE I/O                       │    │
│ │  ┌────────────┐  ┌────────────┐  ┌────────────┐     │    │
│ │  │  Terminal  │  │  Keyboard  │  │   Timer    │     │    │
│ │  │    VGA     │  │    PS/2    │  │    PIT     │     │    │
│ │  └────────────┘  └────────────┘  └────────────┘     │    │
│ └──────────────────────────────────────────────────────┘    │
│                                                              │
│ ┌──────────────────────────────────────────────────────┐    │
│ │          SISTEMA DE INTERRUPCIONES                   │    │
│ │  ┌────────────┐  ┌────────────┐  ┌────────────┐     │    │
│ │  │    IDT     │  │    ISRs    │  │    IRQs    │     │    │
│ │  │ (256 ent.) │  │ (CPU exc.) │  │  (Hardware)│     │    │
│ │  └────────────┘  └────────────┘  └────────────┘     │    │
│ └──────────────────────────────────────────────────────┘    │
│                                                              │
│ ┌──────────────────────────────────────────────────────┐    │
│ │                KERNEL CORE                           │    │
│ │  • GDT (Global Descriptor Table)                     │    │
│ │  • Sistema de pánico y assertions                    │    │
│ │  • Funciones de utilidad (string, print)            │    │
│ └──────────────────────────────────────────────────────┘    │
└──────────────────────────────────────────────────────────────┘
                     │
┌────────────────────┴────────────────────────────────────────┐
│                   CAPA DE HARDWARE                           │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐    │
│  │   CPU    │  │   RAM    │  │   VGA    │  │ Keyboard │    │
│  │  x86-64  │  │          │  │ 0xB8000  │  │  Port 60 │    │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘    │
└──────────────────────────────────────────────────────────────┘
```

---

## 🔄 Flujo de Ejecución

### 1. Boot Sequence

```
Power On
    │
    ├─► BIOS/UEFI
    │       │
    │       ├─► Buscar dispositivo booteable
    │       └─► Cargar GRUB
    │
    ├─► GRUB (Bootloader)
    │       │
    │       ├─► Leer grub.cfg
    │       ├─► Cargar kernel en memoria (0x100000)
    │       ├─► Parsear Multiboot2 header
    │       └─► Saltar a entry point (boot.asm)
    │
    ├─► boot.asm (32-bit Protected Mode)
    │       │
    │       ├─► Verificar Multiboot2 magic
    │       ├─► Guardar puntero a Multiboot info
    │       ├─► Verificar CPUID y Long Mode support
    │       ├─► Configurar paginación (PML4, PDPT, PD)
    │       ├─► Configurar GDT
    │       ├─► Habilitar PAE (CR4)
    │       ├─► Configurar Long Mode (EFER)
    │       ├─► Habilitar paging (CR0)
    │       ├─► Cargar GDT de 64-bit
    │       ├─► Saltar a código de 64-bit
    │       └─► Configurar stack
    │
    ├─► kernel_main() (64-bit Long Mode)
    │       │
    │       ├─► terminal_initialize()
    │       ├─► Mostrar banner
    │       ├─► Verificar Multiboot2 magic
    │       ├─► memory_detect() - Parsear memory map
    │       ├─► pmm_init() - Inicializar PMM
    │       ├─► heap_init() - Inicializar heap
    │       ├─► idt_install() - Configurar IDT
    │       ├─► isr_install() - Instalar ISRs/IRQs
    │       ├─► timer_install() - Configurar timer
    │       ├─► keyboard_install() - Configurar teclado
    │       ├─► shell_init() - Inicializar shell
    │       ├─► Preguntar por tests
    │       └─► shell_run() - Loop principal
    │
    └─► Loop infinito (hlt)
```

---

## 🧠 Gestión de Memoria

### Layout de Memoria Física

```
0x00000000  ┌────────────────────────────────┐
            │   Real Mode IVT (1KB)          │
0x00000400  ├────────────────────────────────┤
            │   BIOS Data Area (256B)        │
0x00000500  ├────────────────────────────────┤
            │   Área libre (~30KB)           │
0x00007C00  ├────────────────────────────────┤
            │   Bootloader (512B)            │
0x00007E00  ├────────────────────────────────┤
            │   Área libre (~480KB)          │
0x00080000  ├────────────────────────────────┤
            │   EBDA (Extended BIOS)         │
0x0009FC00  ├────────────────────────────────┤
            │   VGA Memory (128KB)           │
0x000A0000  │   - Text: 0xB8000              │
0x000C0000  ├────────────────────────────────┤
            │   Video BIOS ROM               │
0x000C8000  ├────────────────────────────────┤
            │   Área libre                   │
0x000F0000  ├────────────────────────────────┤
            │   System BIOS ROM              │
0x00100000  ├════════════════════════════════┤  ◄── KERNEL_BASE
            │   Kernel Code + Data           │
            │   • boot.asm                   │
            │   • kernel.c                   │
            │   • Drivers                    │
            │   • Librerías                  │
0x00200000  ├────────────────────────────────┤
            │   PMM Bitmap                   │
            │   (tamaño dinámico)            │
0x00300000  ├────────────────────────────────┤  ◄── HEAP_START
            │   Heap del Kernel              │
            │   (1MB, expandible)            │
0x00400000  ├────────────────────────────────┤
            │   Memoria Libre                │
            │   (Gestionada por PMM)         │
            │                                │
            │   ...                          │
            │                                │
0xFFFFFFFF  └────────────────────────────────┘  (4GB en 32-bit)
            (Continúa en 64-bit)
```

### Paginación (Identity Mapping)

```
Virtual Address            Physical Address
0x0000000000000000  ──►   0x0000000000000000
0x0000000000001000  ──►   0x0000000000001000
        ...                      ...
0x0000000040000000  ──►   0x0000000040000000

Estructura:
PML4 (Page Map Level 4)
  └─► PDPT (Page Directory Pointer Table)
        └─► PD (Page Directory)
              └─► Páginas de 2MB (512 entradas)

Cobertura: 1GB de RAM mapeado identity
```

### Physical Memory Manager (PMM)

```c
Estructura interna:

┌─────────────────────────────────────────┐
│         PMM State                        │
├─────────────────────────────────────────┤
│ pmm_bitmap: uint32_t*                   │  ◄── Puntero al bitmap
│ pmm_bitmap_size: uint64_t               │  ◄── Tamaño en uint32_t
│ pmm_total_frames: uint64_t              │  ◄── Total de frames
│ pmm_used_frames: uint64_t               │  ◄── Frames ocupados
│ pmm_memory_base: uint64_t               │  ◄── Inicio del área
│ pmm_memory_size: uint64_t               │  ◄── Tamaño del área
└─────────────────────────────────────────┘

Bitmap (1 bit por frame de 4KB):
Bit 0 = Libre
Bit 1 = Usado

Ejemplo (16 frames):
Frame:  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
Bit:    1  1  1  1  0  0  0  1  1  0  0  0  0  0  0  1
        └──Kernel──┘  └─Libre─┘  └─Used─┘  └──Libre──┘

Operaciones:
• pmm_alloc_frame():
  1. Buscar primer bit en 0
  2. Marcar como 1
  3. Retornar dirección física
  
• pmm_free_frame(addr):
  1. Calcular índice del frame
  2. Marcar bit como 0
```

### Heap del Kernel (kmalloc)

```c
Estructura:

┌─────────────────────────────────────────┐
│         Heap State                       │
├─────────────────────────────────────────┤
│ heap_start: 0x300000                    │
│ heap_end: 0x400000 (1MB)                │
│ heap_current: 0x3xxxxx (va creciendo)   │
│ total_allocated: xxxxxx bytes           │
│ allocation_count: xxx                   │
└─────────────────────────────────────────┘

Layout del Heap:

heap_start (0x300000)
    │
    ├──► [Bloque 1: 64 bytes ]
    ├──► [Bloque 2: 128 bytes]
    ├──► [Padding: 4 bytes]    ◄── Alineación
    ├──► [Bloque 3: 1024 bytes]
    │    ...
    ├──► [Bloque N: xxx bytes]
heap_current ◄────┘
    │
    │    [Memoria libre]
    │
heap_end (0x400000)

Algoritmo (Bump Allocator):
1. Alinear heap_current al múltiplo de 'alignment'
2. Verificar que hay espacio
3. Retornar puntero a heap_current
4. Incrementar heap_current en 'size'
5. Limpiar memoria (memset 0)

Nota: Sin free() en Fase 1 - el heap solo crece
```

---

## 🔔 Sistema de Interrupciones

### Interrupt Descriptor Table (IDT)

```c
Estructura en memoria:

IDT (4096 bytes = 256 entradas × 16 bytes)
┌──────────────────────────────────────┐
│ Entry 0: Division by Zero            │  16 bytes
├──────────────────────────────────────┤
│ Entry 1: Debug                       │  16 bytes
├──────────────────────────────────────┤
│ Entry 2: NMI                         │  16 bytes
├──────────────────────────────────────┤
│  ...                                 │
├──────────────────────────────────────┤
│ Entry 32: Timer (IRQ0)               │  16 bytes
├──────────────────────────────────────┤
│ Entry 33: Keyboard (IRQ1)            │  16 bytes
├──────────────────────────────────────┤
│  ...                                 │
├──────────────────────────────────────┤
│ Entry 255: Reserved                  │  16 bytes
└──────────────────────────────────────┘

Formato de cada entrada (16 bytes):
┌─────────────────────────────────────────────┐
│ offset_low (16 bits)    │ Bits 0-15 de RIP  │
│ selector (16 bits)      │ Code segment      │
│ ist (8 bits)            │ Interrupt Stack   │
│ type_attr (8 bits)      │ Flags             │
│ offset_mid (16 bits)    │ Bits 16-31 de RIP │
│ offset_high (32 bits)   │ Bits 32-63 de RIP │
│ zero (32 bits)          │ Reservado         │
└─────────────────────────────────────────────┘

Type_attr:
  Bit 7: Present (P)
  Bit 6-5: DPL (Descriptor Privilege Level)
  Bit 4: 0 (Storage Segment)
  Bit 3-0: Gate Type
    1110 = Interrupt Gate (interrupts disabled)
    1111 = Trap Gate (interrupts enabled)
```

### Flujo de Interrupción

```
Hardware Event (ej: tecla presionada)
    │
    ├─► Keyboard envía señal a PIC
    │
    ├─► PIC determina IRQ (IRQ1 = 0x21)
    │
    ├─► PIC envía interrupción al CPU
    │
    ├─► CPU:
    │   1. Guarda RFLAGS, CS, RIP en stack
    │   2. Deshabilita interrupciones (IF=0)
    │   3. Busca entrada 0x21 en IDT
    │   4. Salta al handler (irq1_handler en interrupts.asm)
    │
    ├─► irq1_handler (Assembly):
    │   1. Guarda registros (rax, rbx, rcx, ...)
    │   2. Llama a irq_handler_main(irq_num=1)
    │   3. Restaura registros
    │   4. Envía EOI al PIC
    │   5. IRETQ (retorno de interrupción)
    │
    ├─► irq_handler_main() (C):
    │   1. Determina IRQ
    │   2. Llama a handler específico:
    │      - IRQ0 → timer_handler()
    │      - IRQ1 → keyboard_handler()
    │   3. Retorna
    │
    └─► keyboard_handler():
        1. Leer scancode del puerto 0x60
        2. Convertir a ASCII
        3. Agregar a buffer circular
        4. Retornar
```

### PIC (Programmable Interrupt Controller)

```
Configuración después del remapeo:

Master PIC (8259A)           Slave PIC (8259A)
┌──────────────────┐        ┌──────────────────┐
│ Command: 0x20    │        │ Command: 0xA0    │
│ Data: 0x21       │        │ Data: 0xA1       │
├──────────────────┤        ├──────────────────┤
│ IRQ0: Timer      │ INT 32 │ IRQ8:  RTC       │ INT 40
│ IRQ1: Keyboard   │ INT 33 │ IRQ9:  ACPI      │ INT 41
│ IRQ2: Cascade    │───────►│ IRQ10: Free      │ INT 42
│ IRQ3: COM2       │ INT 35 │ IRQ11: Free      │ INT 43
│ IRQ4: COM1       │ INT 36 │ IRQ12: PS/2 Mouse│ INT 44
│ IRQ5: LPT2       │ INT 37 │ IRQ13: FPU       │ INT 45
│ IRQ6: Floppy     │ INT 38 │ IRQ14: Primary HD│ INT 46
│ IRQ7: LPT1       │ INT 39 │ IRQ15: Second HD │ INT 47
└──────────────────┘        └──────────────────┘

Máscara de IRQs:
0 = Habilitado
1 = Deshabilitado

Máscara actual (solo Timer y Keyboard habilitados):
Master: 11111100₂ = 0xFC
Slave:  11111111₂ = 0xFF
```

---

## ⌨️ Driver de Teclado

### Arquitectura

```
Hardware                  Driver                   Application
┌─────────┐             ┌──────────┐             ┌──────────┐
│         │  Scancode  │          │   ASCII     │          │
│ Teclado ├───────────►│  Driver  ├────────────►│  Shell   │
│  PS/2   │  (Port 60) │ Keyboard │   (buffer)  │          │
└─────────┘            └──────────┘             └──────────┘
                             │
                             ├─► Estado de modificadores
                             │   • Shift (izq/der)
                             │   • Ctrl (izq/der)
                             │   • Alt (izq/der)
                             │   • Caps Lock
                             │
                             └─► Buffer circular (256 chars)
```

### Buffer Circular

```c
Buffer de 256 caracteres:

┌───┬───┬───┬───┬───┬───┬───┬───┐
│ H │ o │ l │ a │   │   │   │   │  ...  (256 positions)
└───┴───┴───┴───┴───┴───┴───┴───┘
  ↑               ↑
  │               │
 read_pos      write_pos

Operaciones:
• Write (desde IRQ handler):
  buffer[write_pos++] = char;
  write_pos %= 256;  // Wrap around

• Read (desde aplicación):
  char = buffer[read_pos++];
  read_pos %= 256;
  
Lleno cuando: (write_pos + 1) % 256 == read_pos
Vacío cuando: write_pos == read_pos
```

### Scancode → ASCII

```c
Tabla de conversión (simplificada):

Scancode │ Sin Shift │ Con Shift │ Descripción
─────────┼───────────┼───────────┼─────────────
0x1E     │    'a'    │    'A'    │ Tecla A
0x30     │    'b'    │    'B'    │ Tecla B
0x02     │    '1'    │    '!'    │ Tecla 1
0x03     │    '2'    │    '@'    │ Tecla 2
0x39     │    ' '    │    ' '    │ Espacio
0x1C     │   '\n'    │   '\n'    │ Enter
0x0E     │   '\b'    │   '\b'    │ Backspace

Make code: Tecla presionada (0x01-0x7F)
Break code: Tecla soltada (0x81-0xFF = make + 0x80)
```

---

## 🖥️ Driver VGA

### Memory-Mapped I/O

```
Memoria VGA en 0xB8000 (Text Mode 80x25):

Estructura en memoria:
┌────────────────────────────────────────┐
│ Char 0,0 │ Attr 0,0 │ Char 0,1 │ Attr...│  Fila 0
├────────────────────────────────────────┤
│ Char 1,0 │ Attr 1,0 │ Char 1,1 │ Attr...│  Fila 1
├────────────────────────────────────────┤
│   ...                                  │
├────────────────────────────────────────┤
│ Char 24,79│Attr 24,79│               │  Fila 24
└────────────────────────────────────────┘

Total: 80 × 25 × 2 bytes = 4000 bytes

Cada celda = 2 bytes:
┌──────────┬──────────┐
│ Carácter │ Atributo │
│ (ASCII)  │ (Color)  │
└──────────┴──────────┘

Byte de atributo:
┌────────────┬────────────┐
│ Background │ Foreground │
│  4 bits    │  4 bits    │
└────────────┴────────────┘

Cálculo de offset:
offset = (y × 80 + x) × 2
```

### Scroll

```c
Algoritmo de scroll:

1. Copiar filas 1-24 a filas 0-23:
   for (y = 0; y < 24; y++)
       for (x = 0; x < 80; x++)
           buffer[y][x] = buffer[y+1][x];

2. Limpiar última fila (fila 24):
   for (x = 0; x < 80; x++)
       buffer[24][x] = ' ' + (color << 8);

3. Ajustar posición del cursor:
   cursor_y = 24;
   cursor_x = 0;
```

---

## ⏱️ Driver Timer (PIT)

### Configuración

```
PIT (Programmable Interval Timer - 8253/8254)

Frecuencia base: 1.193182 MHz (1193182 Hz)

Para obtener frecuencia deseada:
divisor = 1193182 / frecuencia_deseada

Ejemplo: 100 Hz
divisor = 1193182 / 100 = 11932

Configuración:
1. Enviar comando al puerto 0x43:
   • Channel 0
   • Access mode: Low/High byte
   • Operating mode: Rate generator
   • Binary mode
   
2. Enviar divisor al puerto 0x40:
   • Low byte primero
   • High byte después

El PIT generará IRQ0 cada 10ms (100 Hz)
```

### Contador de Ticks

```c
Variable global:
static volatile uint64_t timer_ticks = 0;

En cada IRQ0:
timer_ticks++;

Conversión a tiempo:
seconds = timer_ticks / 100;       // 100 Hz = 100 ticks/segundo
milliseconds = timer_ticks * 10;   // 1 tick = 10ms
```

---

## 🐚 Shell

### Parser de Comandos

```c
Ejemplo de input:
"echo Hola Mundo"

Proceso de parsing:
1. Copiar input a buffer temporal
   parse_buffer = "echo Hola Mundo"

2. Separar por espacios:
   argv[0] = "echo"
   argv[1] = "Hola"
   argv[2] = "Mundo"
   argc = 3

3. Buscar comando en tabla:
   shell_commands[] = {
       {"echo", "Repite texto", shell_cmd_echo},
       ...
   };

4. Ejecutar handler:
   shell_cmd_echo(argc, argv);
```

### Estructura de Comando

```c
typedef struct {
    char* name;                  // "help"
    char* description;           // "Muestra ayuda"
    void (*function)(int, char**);  // Handler
} shell_command_t;

Tabla de comandos:
┌─────────┬──────────────────────┬──────────────┐
│ Name    │ Description          │ Function     │
├─────────┼──────────────────────┼──────────────┤
│ "help"  │ "Muestra ayuda"      │ cmd_help()   │
│ "echo"  │ "Repite texto"       │ cmd_echo()   │
│ "clear" │ "Limpia pantalla"    │ cmd_clear()  │
│  ...    │  ...                 │  ...         │
│ NULL    │ NULL                 │ NULL         │  ◄── Fin
└─────────┴──────────────────────┴──────────────┘
```

---

## 🧪 Sistema de Testing

### Suite de Tests

```
Estructura:
┌────────────────────────────────────┐
│      run_test_suite()              │
├────────────────────────────────────┤
│  ├─► Timer Tests                   │
│  │   ├─► test_timer_basic()        │
│  │   └─► test_timer_overflow()     │
│  │                                 │
│  ├─► Keyboard Tests                │
│  │   ├─► test_keyboard_lowercase() │
│  │   ├─► test_keyboard_shift()     │
│  │   ├─► test_keyboard_numbers()   │
│  │   └─► test_keyboard_backspace() │
│  │                                 │
│  ├─► Terminal Tests                │
│  │   ├─► test_terminal_colors()    │
│  │   └─► test_terminal_scroll()    │
│  │                                 │
│  ├─► Memory Tests                  │
│  │   ├─► test_pmm_alloc_free()     │
│  │   ├─► test_pmm_no_overlap()     │
│  │   ├─► test_kmalloc_small()      │
│  │   ├─► test_kmalloc_alignment()  │
│  │   └─► test_kmalloc_oom()        │
│  │                                 │
│  └─► Exception Tests               │
│      ├─► test_divide_by_zero()     │
│      ├─► test_invalid_opcode()     │
│      └─► test_page_fault()         │
└────────────────────────────────────┘
```

---

## 🔄 Ciclo de Vida del Sistema

```
1. BOOT
   ├─► Hardware initialization
   └─► Jump to kernel_main()

2. INIT
   ├─► Initialize subsystems
   │   ├─► Terminal
   │   ├─► Memory (PMM + Heap)
   │   ├─► Interrupts (IDT, ISRs, IRQs)
   │   ├─► Drivers (Timer, Keyboard)
   │   └─► Shell
   └─► Run tests (optional)

3. RUNNING
   ├─► Shell loop
   │   ├─► Display prompt
   │   ├─► Read input (keyboard_getchar)
   │   ├─► Parse command
   │   ├─► Execute command
   │   └─► Repeat
   │
   └─► Background
       └─► Timer interrupts (100 Hz)

4. HALT
   └─► cli; hlt loop (halt command)
```

---

## 📊 Dependencias entre Módulos

```
kernel_main
    ├─► terminal (independiente)
    ├─► memory
    │   ├─► terminal (para logs)
    │   └─► string (memset, memcpy)
    ├─► interrupts
    │   └─► terminal (para panic)
    ├─► timer
    │   └─► interrupts (IRQ0)
    ├─► keyboard
    │   └─► interrupts (IRQ1)
    ├─► shell
    │   ├─► terminal
    │   ├─► keyboard
    │   ├─► timer (uptime)
    │   ├─► memory (meminfo)
    │   └─► string
    └─► test_suite
        └─► (todos los módulos)
```

---
