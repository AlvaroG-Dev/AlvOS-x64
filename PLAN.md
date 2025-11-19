# 📋 PLANIFICACIÓN FASE 1: KERNEL BÁSICO FUNCIONAL

## 🎯 **Objetivo de la Fase 1**

Tener un kernel mínimo pero **sólido** que arranque, maneje interrupciones correctamente y tenga las bases fundamentales bien implementadas. Sin prisa, todo bien probado.

---

## ✅ **COMPLETADO**

### Bloque 1.1: Bootstrap y Modo Protegido ✅

- [x]  Cross-compiler configurado
- [x]  Bootloader Multiboot2
- [x]  Transición de 32-bit a 64-bit (Long Mode)
- [x]  GDT básica
- [x]  Paginación identity-mapped (2MB pages)
- [x]  Stack inicial

### Bloque 1.2: Output Básico ✅

- [x]  Driver VGA text mode
- [x]  Funciones de impresión (putchar, writestring)
- [x]  Colores VGA
- [x]  Scroll de pantalla
- [x]  Backspace básico

### Bloque 1.3: Sistema de Interrupciones ✅

- [x]  IDT completa (256 entradas)
- [x]  ISRs (32 excepciones del CPU)
- [x]  IRQs (16 interrupciones hardware)
- [x]  Remapeo del PIC
- [x]  Handlers en Assembly y C
- [x]  EOI (End of Interrupt)

### Bloque 1.4: Drivers de Input/Output ✅

- [x]  Timer PIT (Programmable Interval Timer)
- [x]  Sistema de ticks
- [x]  Teclado PS/2 con buffer circular
- [x]  Scancodes a ASCII
- [x]  Modificadores (Shift, Ctrl, Alt, Caps Lock)

---

## 🎯 **PAUSA 1: REVISIÓN EXHAUSTIVA** ⏸️

**ACTUAL - Empezar aquí**

**Duración estimada:** 1-2 días de trabajo

### Objetivos:

Asegurarnos de que todo lo implementado funciona perfectamente antes de avanzar.

### 1.1 Testing Manual Completo

- [x]  **Teclado:**
    - Probar todas las letras (a-z, A-Z)
    - Probar números y símbolos
    - Probar Shift + cada tecla
    - Probar Caps Lock
    - Probar Backspace en diferentes posiciones
    - Probar Enter
    - Verificar que Ctrl y Alt no crashean
- [x]  **Timer:**
    - Verificar que cuenta correctamente
    - Dejar corriendo 10+ minutos
    - Verificar que no se desborda
- [x]  **Terminal:**
    - Llenar la pantalla completa
    - Verificar scroll correcto
    - Probar diferentes colores
    - Verificar límites de pantalla

### 1.2 Testing de Excepciones

- [x]  Provocar división por cero intencionalmente
- [x]  Verificar que el handler de excepciones funciona
- [x]  Verificar que muestra información correcta
- [x]  Probar con código inválido (opcode inválido)

### 1.3 Code Review

- [x]  Revisar todos los archivos .c y .h
- [x]  Verificar que no hay magic numbers sin explicar
- [x]  Añadir comentarios donde falten
- [x]  Verificar nombres de variables descriptivos
- [x]  Eliminar código comentado innecesario
- [x]  Verificar indentación consistente

### 1.4 Refactoring Necesario

- [ ]  Separar funciones muy largas (>50 líneas)
- [x]  Extraer constantes a #define
- [x]  Verificar que no hay código duplicado
- [x]  Organizar includes de forma consistente

### 1.5 Documentación Básica

- [x]  Crear README.md con:
- [x]  Comentar cada archivo .h con su propósito
- [x]  Añadir licencia (si aplica)

### 1.6 Mejoras Técnicas

- [x]  Implementar función `panic()` para errores fatales
- [x]  Mejorar handler de excepciones con más información
- [x]  Añadir assert() básico para debugging
- [x]  Implementar funciones de utilidad faltantes:
    - strcmp()
    - strcpy()
    - memcpy()
    - memset()

### 1.7 Makefile Mejorado

- [ ]  Añadir target `debug-gdb` para debugging con GDB
- [ ]  Separar CFLAGS de debug y release
- [ ]  Añadir dependencias automáticas

**Entregable:** Sistema completamente probado, documentado y limpio.

---

## 🚀 **Bloque 1.5: Shell Minimalista**

**Duración estimada:** 2-3 días

### Objetivo:

Un shell extremadamente simple para poder interactuar con el kernel.

### 5.1 Infraestructura del Shell

- [x]  Función `readline()` que lee hasta Enter
    - Buffer de 256 caracteres
    - Mostrar lo que se escribe
    - Manejar backspace correctamente
    - Detectar Enter
- [x]  Función `parse_command()`
    - Separar comando del resto
    - Contar argumentos básicos
    - Máximo 4 argumentos
- [x]  Estructura de comando:

c

````c
  typedef struct {
      char* name;
      char* description;
      void (*function)(int argc, char** argv);
  } shell_command_t;
```

- [x] Loop principal del shell:
```
  prompt > [esperar input]
  prompt > [ejecutar comando]
  prompt > [repetir]
````

### 5.2 Comandos Esenciales (Solo 5)

- [x]  `help` - Listar comandos disponibles
- [x]  `clear` - Limpiar pantalla (llenar con espacios)
- [x]  `echo [texto]` - Imprimir el texto
- [x]  `uptime` - Mostrar segundos desde arranque
- [x]  `halt` - Detener el sistema (`cli; hlt` en loop)

### 5.3 Testing del Shell

- [x]  Probar cada comando
- [x]  Probar comando inexistente (mostrar error)
- [x]  Probar línea vacía (no hacer nada)
- [x]  Probar comando muy largo (truncar)
- [x]  Probar muchos argumentos

**Entregable:** Shell funcional con 5 comandos básicos.

---

## 🎯 **PAUSA 2: CONSOLIDACIÓN DEL SHELL** ⏸️

**Duración estimada:** 1 día

### Objetivos:

- [x]  Probar exhaustivamente el shell
- [x]  Mejorar mensajes de error
- [x]  Añadir prompt personalizado con color
- [x]  Documentar cada comando en el código
- []  Verificar que no hay memory leaks (no aplica aún, solo revisar)

**Entregable:** Shell robusto y bien probado.

---

## 🧠 **Bloque 1.6: Gestión de Memoria BÁSICA**

**Duración estimada:** 3-4 días

### Objetivo:

Sistema mínimo de memoria para poder usar `malloc()` en el kernel.

### 6.1 Detección de Memoria

- [ ]  Leer información de Multiboot2
- [ ]  Parsear mmap (memory map)
- [ ]  Imprimir cantidad de RAM disponible
- [ ]  Comando shell: `meminfo`

### 6.2 Physical Memory Manager (PMM) Simple

**Solo lo básico - sin optimizaciones:**

- [ ]  Bitmap allocator (1 bit = 1 frame de 4KB)
- [ ]  `pmm_init()` - Inicializar bitmap
- [ ]  `pmm_alloc_frame()` - Reservar 1 frame
- [ ]  `pmm_free_frame()` - Liberar 1 frame
- [ ]  Marcar kernel como usado

### 6.3 Heap del Kernel (kmalloc simple)

**Implementación básica - sin free por ahora:**

- [ ]  Área de heap de tamaño fijo (ej: 1MB)
- [ ]  `kmalloc(size)` - Allocator lineal simple
- [ ]  Alineación a 4 bytes
- [ ]  Comando shell: `heapinfo`

**NO implementar aún:**

- ❌ kfree() (viene después)
- ❌ Virtual memory manager complejo
- ❌ Page fault handler avanzado
- ❌ Múltiples heaps

### 6.4 Testing de Memoria

- [ ]  Allocar 100 bloques pequeños
- [ ]  Verificar que no se solapan
- [ ]  Llenar heap hasta el límite
- [ ]  Verificar que falla apropiadamente

**Entregable:** Sistema básico de memoria funcional.

---

## 🎯 **PAUSA 3: REVISIÓN COMPLETA DEL SISTEMA** ⏸️

**Duración estimada:** 1-2 días

### Objetivos Finales de Fase 1:

### 3.1 Testing Integrado

- [ ]  Usar shell para probar todo
- [ ]  Verificar que timer no afecta shell
- [ ]  Verificar que teclado funciona siempre
- [ ]  Probar sistema durante 30+ minutos
- [ ]  No debe haber crashes

### 3.2 Documentación Final

- [ ]  README.md completo y actualizado
- [ ]  Guía de arquitectura del sistema
- [ ]  Comentarios en todo el código
- [ ]  Diagrama de módulos

### 3.3 Code Quality

- [ ]  Sin warnings del compilador
- [ ]  Sin código muerto
- [ ]  Consistencia en estilo
- [ ]  Funciones bien nombradas

### 3.4 Preparación para Fase 2

- [ ]  Lista de TODOs para siguiente fase
- [ ]  Identificar limitaciones actuales
- [ ]  Planear mejoras necesarias