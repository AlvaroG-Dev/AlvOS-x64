# Sistema Operativo de 64 bits - Fase 1

Un kernel básico de 64 bits desarrollado desde cero (bare-metal) para arquitectura x86-64.

## 📋 Características Actuales

### ✅ Implementado
- **Bootloader Multiboot2** - Compatible con GRUB
- **Modo Largo (64-bit)** - Transición completa de 32 a 64 bits
- **Paginación** - Identity mapping con páginas de 2MB
- **Sistema de Interrupciones**
  - IDT completa (256 entradas)
  - 32 excepciones del CPU
  - 16 IRQs de hardware
  - PIC remapeado correctamente
- **Drivers**
  - Terminal VGA (80x25) con colores y scroll
  - Timer PIT (Programmable Interval Timer) a 100Hz
  - Teclado PS/2 con buffer circular
- **Shell Minimalista**
  - Comandos: help, clear, echo, uptime, halt
  - Parser de argumentos básico
  - Prompt interactivo con colores
- **Utilidades**
  - Funciones de string (strcmp, strcpy, memcpy, etc.)
  - Sistema de pánico para errores fatales
  - Suite de pruebas automatizada
  - Sistema de assertions

## 🛠️ Requisitos

### Software Necesario
- **Cross-compiler**: `x86_64-elf-gcc` y `x86_64-elf-ld`
- **Ensamblador**: `nasm`
- **Bootloader**: `grub-mkrescue`, `grub-pc-bin`, `xorriso`
- **Emulador**: `qemu-system-x86_64`
- **Build tools**: `make`

### Instalación en Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential nasm qemu-system-x86 \
                 grub-pc-bin xorriso mtools
```

### Construir Cross-Compiler
Si no tienes `x86_64-elf-gcc`, necesitas construir un cross-compiler.
Consulta: https://wiki.osdev.org/GCC_Cross-Compiler

O usa el script `build-cross-compiler.sh` incluido:
```bash
chmod +x build-cross-compiler.sh
./build-cross-compiler.sh
```

Luego añade a tu `~/.bashrc`:
```bash
export PATH="$HOME/opt/cross/bin:$PATH"
```

## 🚀 Compilación y Ejecución

### Compilar
```bash
make clean
make
```

### Ejecutar en QEMU
```bash
make run
```

### Modo Debug
```bash
make debug          # Con log de interrupciones
make debug-gdb      # Con soporte para GDB (próximamente)
```

### Limpiar
```bash
make clean
```

## 📁 Estructura del Proyecto

```
.
├── boot.asm            # Bootloader (32→64 bit, paginación)
├── constants.h         # Definiciones Generales
├── execption_test.c/h  # Tests de execpciones del CPU
├── interrupts.asm      # Stubs de ISRs/IRQs
├── idt.asm            # Carga de IDT
├── kernel.c           # Punto de entrada del kernel
├── terminal.c/h       # Driver VGA text mode
├── isr.c/h            # Handlers de interrupciones
├── timer.c/h          # Driver PIT timer
├── keyboard.c/h       # Driver de teclado PS/2
├── shell.c/h          # Shell minimalista
├── string.c/h         # Funciones de string/memoria
├── panic.c/h          # Sistema de pánico
├── test_suite.c/h     # Suite de pruebas
├── linker.ld          # Linker script
├── Makefile           # Sistema de construcción
├── README.md          # Este archivo
├── grub.cfg            # Menuentry para GRUB
```

## 🎯 Uso

### Al Iniciar
El sistema arranca y pregunta si deseas ejecutar la suite de pruebas.

**Presiona `s` para ejecutar tests:**
- Tests del timer
- Tests del teclado (interactivos)
- Tests del terminal
- Verificación de colores y scroll
- Excepciones del CPU

**Presiona n para ir directamente al shell:**

```bash
Sistema listo. Iniciando shell...
os> 
Comandos del Shell
help - Muestra lista de comandos disponibles

clear - Limpia la pantalla

echo [texto] - Repite el texto ingresado

uptime - Muestra segundos transcurridos desde el boot

halt - Detiene el sistema de forma segura

```

## Ejemplos de Uso

```bash
os> echo Hola Mundo!
Hola Mundo!

os> uptime
Tiempo activo: 125 segundos

os> clear

os> help
Comandos disponibles:
  help    - Muestra esta ayuda
  clear   - Limpia la pantalla  
  echo    - Repite el texto
  uptime  - Muestra tiempo activo
  halt    - Detiene el sistema
```

### Suite de Pruebas
La suite incluye:
1. ✅ **Timer básico** - Verifica conteo de 3 segundos
2. ✅ **Teclado minúsculas** - Test interactivo
3. ✅ **Teclado Shift** - Test interactivo  
4. ✅ **Teclado números** - Test interactivo
5. ✅ **Backspace** - Verificación visual
6. ✅ **Colores del terminal** - Verificación visual
7. ✅ **Scroll del terminal** - Verificación visual
8. ✅ **Excepciones del CPU** - Verificación visual por pantalla de PANIC

## 🔧 Características Técnicas

### Bootloader
- Multiboot2 compliant
- Verifica soporte de CPUID y Long Mode
- Configura paginación (4 niveles)
- Configura GDT básica
- Transición a 64-bit

### Gestión de Interrupciones
- IDT de 64-bit
- Handlers para todas las excepciones del CPU
- PIC remapeado (IRQ 0-7 → INT 32-39, IRQ 8-15 → INT 40-47)
- EOI (End of Interrupt) correcto
- Pantalla de PANIC

### Timer
- Frecuencia: 100Hz (10ms por tick)
- Contador de ticks desde boot
- Función de espera: `timer_wait(ms)`

### Teclado
- Scancode Set 1 (US layout)
- Buffer circular de 256 caracteres
- Soporte para Shift, Ctrl, Alt, Caps Lock
- Funciones bloqueantes y no-bloqueantes

### Terminal VGA
- Resolución: 80x25 caracteres
- 16 colores
- Scroll automático
- Soporte para backspace y newline

### Shell
- Buffer de entrada de 256 caracteres
- Soporte para backspace en tiempo real
- Parser de argumentos básico
- Prompt con colores personalizables

## 🐛 Debugging

### Provocar Excepciones para Testing
Puedes modificar `kernel_main()` para probar el handler de excepciones:

```c
// División por cero
int x = 5 / 0;

// Opcode inválido
__asm__ volatile ("ud2");

// Acceso a memoria inválida
volatile int *ptr = (int*)0xDEADBEEF;
*ptr = 42;
```

### Verificar Interrupciones en QEMU
```bash
make debug
```
Verás líneas como:
```
Servicing hardware INT=0x20    # Timer (IRQ0)
Servicing hardware INT=0x21    # Keyboard (IRQ1)
```

## 📊 Estado del Proyecto

### Fase 1: Kernel Básico ✅
- [x] Bootstrap y Long Mode
- [x] Sistema de interrupciones
- [x] Drivers básicos (Timer, Teclado, VGA)
- [x] Funciones de utilidad
- [x] Sistema de testing
- [x] Shell minimalista (próximo)
- [ ] Gestión de memoria básica (próximo)

### Fase 2: Expansión (Planeado)
- [ ] Sistema de archivos virtual (VFS)
- [ ] Gestión de memoria avanzada
- [ ] Multitarea cooperativa
- [ ] Syscalls básicas

### Fase 3: Avanzado (Futuro)
- [ ] Multitarea preemptiva
- [ ] Modo usuario
- [ ] Drivers avanzados
- [ ] Red básica

## 📚 Referencias

- [OSDev Wiki](https://wiki.osdev.org/)
- [Intel 64 and IA-32 Architectures Software Developer's Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [Multiboot2 Specification](https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html)

## 📝 Notas

- Este es un proyecto educativo
- Diseñado para aprender desarrollo de OS

## 🤝 Contribuciones

Este es un proyecto de aprendizaje personal. Si encuentras bugs o tienes sugerencias, siéntete libre de reportarlos.

## 📄 Licencia


                    GNU GENERAL PUBLIC LICENSE
                       Version 3, 29 June 2007

 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
 Everyone is permitted to copy and distribute verbatim copies
 of this license document, but changing it is not allowed.


---

**Última actualización**: Fase 1 - Revisión completa (PAUSA 1)
