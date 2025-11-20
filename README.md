# 🖥️ AlvOS - Sistema Operativo de 64 bits

Un kernel de 64 bits desarrollado desde cero (bare-metal) para arquitectura x86-64.  
**Fase 1 Completa** - Kernel básico funcional con gestión de memoria.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Architecture](https://img.shields.io/badge/arch-x86__64-orange.svg)]()
[![Status](https://img.shields.io/badge/status-Fase%201%20Completa-brightgreen.svg)]()

---

## 📋 Características Actuales

### ✅ **Sistema Base**
- **Bootloader Multiboot2** - Compatible con GRUB2
- **Modo Largo (64-bit)** - Transición completa de 32 a 64 bits
- **Paginación** - Identity mapping con páginas de 2MB
- **GDT** - Global Descriptor Table configurada
- **Stack** - 16KB para el kernel

### ✅ **Sistema de Interrupciones**
- **IDT completa** - 256 entradas (64-bit)
- **32 excepciones del CPU** - Todas manejadas con información detallada
- **16 IRQs de hardware** - PIC remapeado correctamente
- **Handlers** - Assembly stubs + handlers en C
- **EOI automático** - End of Interrupt correcto

### ✅ **Drivers de Hardware**
- **Terminal VGA** - 80x25, 16 colores, scroll automático
- **Timer PIT** - Programmable Interval Timer a 100Hz
- **Teclado PS/2** - Buffer circular, modificadores (Shift, Ctrl, Alt, Caps Lock)

### ✅ **Gestión de Memoria**
- **Detección automática** - Vía Multiboot2 memory map
- **PMM (Physical Memory Manager)** - Bitmap allocator para frames de 4KB
- **Heap del Kernel** - Allocator lineal simple (kmalloc)
- **Información detallada** - Comandos `meminfo` y `heapinfo`

### ✅ **Shell Interactivo**
- **8 comandos funcionales**
  - `help` - Lista de comandos con descripciones detalladas
  - `clear` - Limpia la pantalla
  - `echo` - Repite texto
  - `uptime` - Tiempo activo del sistema
  - `meminfo` - Información de memoria física
  - `heapinfo` - Información del heap del kernel
  - `history` - Historial de comandos
  - `halt` - Detiene el sistema de forma segura
  - `reboot` - Reinicia el sistema
- **Parser avanzado** - Soporta comillas y múltiples argumentos
- **Historial** - Guarda últimos 10 comandos
- **Prompt personalizado** - Con colores

### ✅ **Sistema de Testing**
- **Suite automatizada** - Tests de timer, teclado, terminal
- **Tests de memoria** - Verificación de PMM y heap
- **Tests de excepciones** - División por cero, opcode inválido, etc.
- **Sistema de PANIC** - Para errores fatales con información de debug

### ✅ **Utilidades**
- **Funciones de string** - strcmp, strcpy, strcat, strchr, strlen
- **Funciones de memoria** - memcpy, memset, memcmp, memmove
- **Assertions** - Sistema de assert para debugging
- **Impresión** - Funciones para hex, decimal, strings con colores

---

## 🛠️ Requisitos

### Software Necesario

| Herramienta | Versión Mínima | Propósito |
|------------|----------------|-----------|
| x86_64-elf-gcc | 10.0+ | Cross-compiler para kernel |
| x86_64-elf-ld | 2.35+ | Linker |
| nasm | 2.14+ | Ensamblador |
| grub-mkrescue | 2.04+ | Crear imagen ISO |
| xorriso | 1.5+ | Herramienta para ISOs |
| qemu-system-x86_64 | 5.0+ | Emulador |
| make | 4.0+ | Build system |

### Instalación en Ubuntu/Debian

```bash
# Actualizar repositorios
sudo apt update

# Instalar herramientas
sudo apt install build-essential nasm qemu-system-x86 \
                 grub-pc-bin xorriso mtools
```

### Construir Cross-Compiler

Si no tienes `x86_64-elf-gcc`, necesitas construir un cross-compiler:

```bash
# Descargar binutils y gcc
mkdir ~/cross
cd ~/cross
wget https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz

# Descomprimir
tar xzf binutils-2.41.tar.gz
tar xzf gcc-13.2.0.tar.gz

# Variables
export PREFIX="$HOME/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

# Compilar binutils
cd binutils-2.41
mkdir build && cd build
../configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install

# Compilar gcc
cd ../../gcc-13.2.0
mkdir build && cd build
../configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make install-gcc
make install-target-libgcc
```

Añade a tu `~/.bashrc`:
```bash
export PATH="$HOME/opt/cross/bin:$PATH"
```

**Referencia completa**: [OSDev GCC Cross-Compiler](https://wiki.osdev.org/GCC_Cross-Compiler)

---

## 🚀 Compilación y Ejecución

### Compilar el Kernel

```bash
# Limpiar compilación anterior
make clean

# Compilar todo
make

# Resultado: myos.iso
```

### Ejecutar en QEMU

```bash
# Modo normal
make run

# Con 256MB de RAM
qemu-system-x86_64 -cdrom myos.iso -m 256M

# Modo debug (con logs de interrupciones)
make debug
```

### Debugging Avanzado

```bash
# Con GDB (terminal 1)
make debug-gdb

# Conectar GDB (terminal 2)
gdb kernel.elf
(gdb) target remote localhost:1234
(gdb) break kernel_main
(gdb) continue
```

### Limpiar Build

```bash
make clean
```

---

## 📁 Estructura del Proyecto

```
AlvOS/
├── boot/
│   ├── boot.asm              # Bootloader (32→64 bit)
│   ├── interrupts.asm        # ISR/IRQ stubs
│   └── idt.asm              # Carga de IDT
├── include/
│   ├── constants.h           # Constantes globales
│   ├── multiboot_header.h    # Definiciones Multiboot2
│   ├── terminal.h            # Driver VGA
│   ├── isr.h                # Sistema de interrupciones
│   ├── timer.h              # Driver timer
│   ├── keyboard.h           # Driver teclado
│   ├── memory.h             # Gestión de memoria
│   ├── kmalloc.h            # Heap del kernel
│   ├── shell.h              # Shell interactivo
│   ├── string.h             # Funciones de string
│   ├── panic.h              # Sistema de pánico
│   ├── test_suite.h         # Suite de tests
│   └── exception_test.h     # Tests de excepciones
├── src/
│   ├── kernel.c             # Punto de entrada
│   ├── terminal.c           # Implementación VGA
│   ├── isr.c               # Handlers de interrupciones
│   ├── timer.c             # Implementación timer
│   ├── keyboard.c          # Implementación teclado
│   ├── memory.c            # PMM y detección
│   ├── kmalloc.c           # Heap allocator
│   ├── shell.c             # Implementación shell
│   ├── string.c            # Implementación strings
│   ├── panic.c             # Sistema de pánico
│   ├── test_suite.c        # Suite de tests
│   ├── exception_test.c    # Tests de excepciones
│   └── memory_tests.c      # Tests de memoria
├── linker.ld                # Linker script
├── Makefile                 # Build system
├── grub.cfg                 # Configuración GRUB
├── README.md                # Este archivo
├── TESTING_CHECKLIST.md     # Checklist de testing manual
└── LICENSE                  # GPL v3

```

---

## 🎯 Guía de Uso

### Al Iniciar el Sistema

1. **Banner de bienvenida**
   - Se muestra información del sistema
   - Se verifica magic number de Multiboot2
   - Se inicializan todos los subsistemas

2. **Detección de memoria**
   - Se parsea el memory map del bootloader
   - Se inicializa el PMM (Physical Memory Manager)
   - Se inicializa el heap del kernel

3. **Pregunta de testing**
   ```
   ¿Ejecutar suite de pruebas? (s/n):
   ```
   - **s** → Ejecuta todos los tests automáticos
   - **n** → Va directo al shell

### Usando el Shell

#### Comandos Básicos

```bash
# Ver ayuda general
os> help

# Ver ayuda de un comando específico
os> help echo

# Limpiar pantalla
os> clear

# Imprimir texto
os> echo Hola Mundo!

# Ver tiempo activo
os> uptime
```

#### Comandos de Memoria

```bash
# Ver información de memoria física
os> meminfo
💾 INFORMACIÓN DE MEMORIA
=========================
Memoria total detectada: 256 MB
Memoria disponible: 255 MB
Regiones de memoria: 6

🧮 GESTOR DE MEMORIA FÍSICA (PMM)
=================================
Área gestionada: 0x100000 - 0x10000000
Frames totales: 65280 (255 MB)
Frames usados: 1024 (4 MB)
Frames libres: 64256 (251 MB)
Uso: 1%

# Ver información del heap
os> heapinfo
🗄️ INFORMACIÓN DEL HEAP
=========================
Rango: 0x300000 - 0x400000
Tamaño total: 1024 KB
Usado: 128 KB
Disponible: 896 KB
Allocaciones: 42
Uso: 12%
```

#### Historial

```bash
# Ver comandos anteriores
os> history

# Navegar historial (próximamente)
# Usar flechas arriba/abajo
```

#### Detener/Reiniciar

```bash
# Detener sistema (halt)
os> halt
DETENIENDO EL SISTEMA
=======================
El sistema se está apagando de forma segura...

# Reiniciar (simula reinicio del shell)
os> reboot
```

---

## 🧪 Suite de Pruebas

### Tests Automáticos

Al responder **s** en el boot:

1. ✅ **Timer básico** - Cuenta 3 segundos
2. ✅ **Teclado interactivo** - Minúsculas, Shift, números, backspace
3. ✅ **Terminal** - Colores y scroll
4. ✅ **Excepciones** - División por cero, opcode inválido
5. ✅ **Memoria PMM** - Allocar/liberar frames
6. ✅ **Memoria Heap** - Kmalloc múltiples bloques

### Testing Manual

Consulta `TESTING_CHECKLIST.md` para una guía completa de testing manual que incluye:
- Tests de estabilidad (30+ minutos)
- Tests de cada comando del shell
- Tests de límites del teclado y terminal
- Verificación de memoria
- Casos edge

---

## 🔧 Características Técnicas Detalladas

### Bootloader (boot.asm)

```nasm
- Multiboot2 compliant
- Verifica CPUID y Long Mode support
- Configura paginación de 4 niveles:
  * PML4 (Page Map Level 4)
  * PDPT (Page Directory Pointer Table)
  * PD (Page Directory) - 2MB pages
- GDT con segmentos code/data de 64-bit
- Transición a Long Mode
- Stack de 16KB
```

### Sistema de Interrupciones

```
IDT de 256 entradas (16 bytes cada una)
├── 0-31:   Excepciones del CPU
│   ├── 0:  Division by Zero
│   ├── 6:  Invalid Opcode
│   ├── 13: General Protection Fault
│   └── 14: Page Fault
├── 32-47:  IRQs de hardware (remapeadas)
│   ├── 32: Timer (IRQ0)
│   ├── 33: Keyboard (IRQ1)
│   └── ...
└── 48-255: Reservadas para futuro uso
```

### PIC (Programmable Interrupt Controller)

```
Master PIC: IRQ 0-7  → INT 32-39
Slave PIC:  IRQ 8-15 → INT 40-47

ICW1: Inicialización
ICW2: Vector offset
ICW3: Cascade configuration
ICW4: Modo 8086
```

### Timer (PIT - Programmable Interval Timer)

```
Frecuencia base: 1.193182 MHz
Divisor configurado: 11932 (para 100Hz)
Frecuencia resultante: 100 Hz (10ms por tick)

Función de espera:
timer_wait(ms) = esperar (ms * 100 / 1000) ticks
```

### Teclado PS/2

```
Scancode Set 1 (US layout)
Buffer circular de 256 caracteres

Estados:
- Shift (izq/der)
- Ctrl (izq/der)
- Alt (izq/der)
- Caps Lock

Conversión: Scancode → ASCII con tabla de mapeo
```

### Terminal VGA (Text Mode)

```
Memoria VGA: 0xB8000
Resolución: 80x25 caracteres
Formato: [Carácter][Atributo] (16 bits)

Atributo = [Background 4 bits][Foreground 4 bits]

16 colores disponibles:
0=Negro, 1=Azul, 2=Verde, 3=Cyan,
4=Rojo, 5=Magenta, 6=Marrón, 7=Gris Claro,
8=Gris Oscuro, 9=Azul Claro, 10=Verde Claro, 11=Cyan Claro,
12=Rojo Claro, 13=Magenta Claro, 14=Amarillo, 15=Blanco
```

### Gestión de Memoria

#### Physical Memory Manager (PMM)

```c
Algoritmo: Bitmap allocator
Granularidad: 4KB (1 page frame)
Bitmap: 1 bit por frame

Funciones:
- pmm_init(base, size)      // Inicializar
- pmm_alloc_frame()          // Reservar frame
- pmm_free_frame(addr)       // Liberar frame
- pmm_get_free_memory()      // Memoria libre
```

#### Heap del Kernel (kmalloc)

```c
Algoritmo: Bump allocator (lineal simple)
Tamaño inicial: 1MB
Ubicación: 0x300000 (3MB)
Alineación: 4 bytes (configurable)

Funciones:
- heap_init()                     // Inicializar
- kmalloc(size)                   // Allocar memoria
- kmalloc_aligned(size, align)    // Allocar alineado

Nota: Sin kfree() por ahora (Fase 1)
```

---

## 🐛 Debugging y Troubleshooting

### Provocar Excepciones para Testing

Puedes modificar temporalmente `kernel.c` para probar handlers:

```c
// División por cero
void test_exception(void) {
    int x = 5;
    int y = 0;
    int z = x / y;  // Trigger INT 0
}

// Opcode inválido
void test_invalid_opcode(void) {
    __asm__ volatile ("ud2");  // Trigger INT 6
}

// Page fault
void test_page_fault(void) {
    volatile int *ptr = (int*)0xDEADBEEF;
    *ptr = 42;  // Trigger INT 14
}
```

### Ver Interrupciones en QEMU

```bash
make debug
```

Verás logs como:
```
check_exception old: 0xffffffff new 0xe
     0: v=0e e=0002 i=0 cpl=0 IP=0008:0000000000101234 ...
Servicing hardware INT=0x20    # Timer
Servicing hardware INT=0x21    # Keyboard
```

### Debugging con GDB

```bash
# Terminal 1: Iniciar QEMU con GDB server
qemu-system-x86_64 -cdrom myos.iso -s -S

# Terminal 2: Conectar GDB
gdb kernel.elf
(gdb) target remote localhost:1234
(gdb) break kernel_main
(gdb) continue
(gdb) info registers
(gdb) x/10i $rip
```

### Problemas Comunes

#### El sistema no arranca (pantalla negra)

**Posibles causas:**
1. Cross-compiler incorrecto → Usar `x86_64-elf-gcc`
2. GRUB mal configurado → Verificar `grub.cfg`
3. Multiboot2 header incorrecto → Ver `boot.asm`

#### "Error: Magic number incorrecto"

**Solución:**
- Verificar que GRUB está pasando información correcta
- El bootloader debe dejar `eax = 0x36d76289`

#### Excepciones al azar

**Posibles causas:**
1. Stack overflow → Aumentar `STACK_SIZE` en `constants.h`
2. Paginación incorrecta → Verificar `boot.asm`
3. Memory corruption → Revisar bounds de arrays

#### Timer demasiado rápido/lento

**Solución:**
```c
// En timer.c, ajustar frecuencia:
timer_install(100);  // 100Hz = 10ms por tick
timer_install(1000); // 1000Hz = 1ms por tick
```

---

## 📊 Estado del Proyecto

### ✅ Fase 1: Kernel Básico (COMPLETA)

#### Bloque 1.1: Bootstrap ✅
- [x] Cross-compiler configurado
- [x] Bootloader Multiboot2
- [x] Transición 32→64 bit
- [x] GDT y paginación
- [x] Stack inicial

#### Bloque 1.2: Output ✅
- [x] Driver VGA
- [x] Colores y scroll
- [x] Funciones de impresión

#### Bloque 1.3: Interrupciones ✅
- [x] IDT completa
- [x] ISRs (excepciones)
- [x] IRQs (hardware)
- [x] PIC remapeado

#### Bloque 1.4: Drivers ✅
- [x] Timer PIT
- [x] Teclado PS/2
- [x] Buffer circular
- [x] Modificadores

#### Bloque 1.5: Shell ✅
- [x] Readline
- [x] Parser de comandos
- [x] 9 comandos funcionales
- [x] Historial

#### Bloque 1.6: Memoria ✅
- [x] Detección de RAM
- [x] PMM (Physical Memory Manager)
- [x] Heap del kernel (kmalloc)
- [x] Comandos meminfo/heapinfo

#### Testing Completo ✅
- [x] Suite automatizada
- [x] Tests manuales
- [x] Estabilidad verificada
- [x] Documentación completa

---

### 🚀 Fase 2: Expansión (PRÓXIMA)

#### Memoria Avanzada
- [ ] Virtual Memory Manager (VMM)
- [ ] Page fault handler
- [ ] kfree() - Liberar memoria del heap
- [ ] Heap dinámico expandible
- [ ] Memory pools

#### Sistema de Archivos
- [ ] VFS (Virtual File System)
- [ ] Initrd/initramfs básico
- [ ] Lectura de archivos

#### Procesos
- [ ] Estructuras de proceso
- [ ] Context switching básico
- [ ] Scheduler round-robin

#### Syscalls
- [ ] Interfaz de syscalls
- [ ] Syscalls básicas (read, write, exit)

---

### 🌟 Fase 3: Avanzado (FUTURO)

- [ ] Multitarea preemptiva
- [ ] Modo usuario (ring 3)
- [ ] ELF loader
- [ ] Fork/exec
- [ ] Pipes y redirección
- [ ] Driver ATA/IDE
- [ ] Sistema de archivos ext2
- [ ] Red básica (e1000)
- [ ] Stack TCP/IP simple

---

## 📚 Referencias y Recursos

### Documentación Oficial

- **[OSDev Wiki](https://wiki.osdev.org/)** - La biblia del OS development
- **[Intel SDM](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)** - Manual completo de x86-64
- **[AMD64 Architecture](https://www.amd.com/en/support/tech-docs)** - Documentación AMD
- **[Multiboot2 Spec](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html)** - Especificación oficial

### Tutoriales y Guías

- [OSDev Bare Bones](https://wiki.osdev.org/Bare_Bones)
- [Writing a Simple OS from Scratch](https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf)
- [The little book about OS development](https://littleosbook.github.io/)

### Libros Recomendados

- **"Operating Systems: Three Easy Pieces"** - Arpaci-Dusseau
- **"Modern Operating Systems"** - Andrew Tanenbaum
- **"Operating System Concepts"** - Silberschatz, Galvin, Gagne

### Comunidades

- [r/osdev](https://reddit.com/r/osdev) - Reddit
- [OSDev Forums](https://forum.osdev.org/)
- [OSDev Discord](https://discord.gg/RnCtsqD)

---

## 🤝 Contribuciones

Este es un proyecto educacional de aprendizaje personal. Si encuentras bugs o tienes sugerencias:

1. **Reportar bugs**: Abre un issue describiendo el problema
2. **Sugerencias**: Ideas para mejoras son bienvenidas
3. **Pull requests**: Se aceptan si mantienen la calidad del código

### Estilo de Código

- **Indentación**: 4 espacios
- **Naming**: snake_case para funciones, UPPER_CASE para macros
- **Comentarios**: Explicar el "por qué", no el "qué"
- **Headers**: Todos los .c deben tener su .h correspondiente

---

## 📄 Licencia

```
GNU GENERAL PUBLIC LICENSE
Version 3, 29 June 2007

Copyright (C) 2024 [Tu Nombre]

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

---

## 📝 Notas del Desarrollador

### Por qué un allocator lineal sin free?

En la Fase 1, usamos un bump allocator (lineal) sin `kfree()` por simplicidad:

**Ventajas:**
- Extremadamente simple de implementar
- Muy rápido (O(1))
- Sin fragmentación
- Perfecto para estructuras que viven todo el runtime del kernel

**Limitación:**
- No se puede liberar memoria individual
- El heap crece monotónicamente

**Solución en Fase 2:**
Implementaremos un allocator más sofisticado (slab allocator o buddy allocator) con soporte para `kfree()`.

### Por qué identitiy mapping?

La paginación actual mapea memoria 1:1 (dirección virtual = física) porque:
- Es simple de implementar
- Suficiente para un kernel monolítico pequeño
- No necesitamos protección de memoria aún

En Fase 2 implementaremos paginación completa con:
- Higher-half kernel
- Copy-on-write
- Demand paging
- Y mas...!
---

## 🎯 Objetivos de Aprendizaje Logrados

Al completar la Fase 1, has aprendido:

✅ Cómo funciona el boot process de x86-64
✅ Transición de 32-bit a 64-bit (Long Mode)
✅ Sistema de interrupciones y excepciones
✅ Drivers básicos de hardware (timer, teclado, VGA)
✅ Gestión básica de memoria (PMM, heap allocator)
✅ Parsing y ejecución de comandos
✅ Testing y debugging de sistemas bare-metal
✅ Estructura de un kernel modular

---

## 🏆 Agradecimientos

- **OSDev Community** - Por la documentación increíble
- **Intel/AMD** - Por los manuales técnicos detallados
- **QEMU Team** - Por el excelente emulador
- **GNU Project** - Por las herramientas de desarrollo

---

**¡Gracias por explorar AlvOS!** 🚀

Si tienes preguntas o encuentras problemas, no dudes en abrir un issue.

---

**Última actualización**: Fase 1 Completa - [20/11/2025]  
**Versión**: 1.0.0  
**Estado**: ✅ Estable y probado