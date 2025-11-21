# 📋 PLANIFICACIÓN FASE 2: MULTITAREA Y GESTIÓN AVANZADA DE MEMORIA

## 🎯 Objetivo de la Fase 2
Expandir el kernel con gestión avanzada de memoria (incluyendo `kfree()`), virtual memory manager completo, y multitarea básica con scheduler. Al final de esta fase, tendremos múltiples procesos ejecutándose concurrentemente.

---

## ⚙️ Bloque 2.1: Gestión Avanzada de Memoria Física

**Duración estimada:** 3-4 días

**Objetivo:** Mejorar el PMM actual y añadir funcionalidad de liberación de memoria.

### 1.1 Mejoras al Physical Memory Manager
- [x] Implementar `pmm_alloc_frames(count)` para múltiples frames
- [x] Implementar `pmm_free_frames(addr, count)` para múltiples frames
- [x] Añadir estadísticas de memoria:
  - Total frames disponibles
  - Frames usados
  - Frames libres
  - Porcentaje de uso
- [x] Función `pmm_print_stats()` para debugging
- [x] Mejorar bitmap con búsqueda optimizada de bloques contiguos

### 1.2 Heap Allocator con Free
Implementar un heap allocator real con soporte para `kfree()`:

- [x] Estructura de bloque de heap:
```c
typedef struct heap_block {
    size_t size;
    bool is_free;
    struct heap_block* next;
} heap_block_t;
```
- [x] `kmalloc(size)` mejorado con linked list
- [x] `kfree(ptr)` para liberar memoria
- [x] `krealloc(ptr, new_size)` para redimensionar
- [x] Coalescencia de bloques libres adyacentes
- [x] Split de bloques grandes

### 1.3 Testing de Memoria Avanzado
- [x] Allocar y liberar 1000+ bloques aleatorios
- [x] Verificar que no hay fragmentación excesiva
- [x] Test de coalescencia (alloc-free-alloc)
- [x] Comando shell: `memtest` (ejecuta suite de tests)
- [x] Verificar que `kfree()` de puntero inválido no crashea

**Entregable:** Sistema de memoria robusto con allocation/free funcional.

---

## 🗺️ Bloque 2.2: Virtual Memory Manager (VMM)

**Duración estimada:** 4-5 días

**Objetivo:** Implementar paginación completa con espacios de direcciones separados por proceso.

### 2.1 Estructuras de Paginación
- [ ] Funciones para manipular page tables:
  - `pml4_t* vmm_create_address_space()` - Nueva PML4
  - `void vmm_destroy_address_space(pml4_t*)` - Liberar espacio
  - `void vmm_map_page(pml4_t*, virt, phys, flags)` - Mapear página
  - `void vmm_unmap_page(pml4_t*, virt)` - Desmapear página
  - `phys_addr_t vmm_get_physical(pml4_t*, virt)` - Traducir dirección

### 2.2 Kernel Memory Space
- [ ] Crear espacio de memoria del kernel separado
- [ ] Mapear kernel en higher-half (ej: 0xFFFFFFFF80000000)
- [ ] Mantener identity-mapping en zona baja para compatibilidad
- [ ] Mapear heap del kernel
- [ ] Mapear hardware (VGA, etc.)

### 2.3 Page Fault Handler
- [ ] ISR para page fault (excepción #14)
- [ ] Leer CR2 (dirección que causó fault)
- [ ] Leer error code (presente, write, user, etc.)
- [ ] Mostrar información detallada del fault:
  - Dirección virtual
  - Tipo de acceso (read/write/execute)
  - Modo (kernel/user)
  - Instruction pointer
- [ ] Comando shell: `pagefault` (provoca fault para testing)

### 2.4 Testing de VMM
- [ ] Crear y destruir múltiples address spaces
- [ ] Mapear/unmapear muchas páginas
- [ ] Verificar traducción virtual→física correcta
- [ ] Provocar page faults intencionalmente
- [ ] Comando shell: `vmtest` (suite de tests de VM)

**Entregable:** Sistema completo de memoria virtual funcional.

---

## 🎯 PAUSA 1: REVISIÓN DE MEMORIA ⏸️

**Duración estimada:** 1-2 días

### Objetivos de la Pausa:
- [ ] Verificar que no hay memory leaks en el sistema
- [ ] Probar allocaciones masivas (1000+ allocs)
- [ ] Verificar que VMM funciona en todos los casos
- [ ] Code review completo del subsistema de memoria
- [ ] Documentar estructuras de datos y algoritmos
- [ ] Actualizar diagramas de arquitectura
- [ ] Corregir cualquier bug encontrado
- [ ] Optimizar código si es necesario

**Entregable:** Subsistema de memoria sólido y bien documentado.

---

## 🔄 Bloque 2.3: Infraestructura de Multitarea

**Duración estimada:** 4-5 días

**Objetivo:** Implementar estructuras para procesos y context switching básico.

### 3.1 Estructura de Proceso (PCB)
```c
typedef struct process {
    uint32_t pid;                    // Process ID
    char name[32];                   // Nombre del proceso
    
    // Context
    registers_t registers;           // Todos los registros (RAX, RBX, etc.)
    uint64_t rip;                    // Instruction pointer
    uint64_t rsp;                    // Stack pointer
    uint64_t rbp;                    // Base pointer
    
    // Memory
    pml4_t* page_directory;          // Espacio de memoria virtual
    void* kernel_stack;              // Stack del kernel para este proceso
    
    // State
    enum process_state state;        // RUNNING, READY, BLOCKED, etc.
    uint32_t priority;               // Prioridad (0-3)
    uint64_t time_slice;             // Quantum de tiempo restante
    
    // Linked list
    struct process* next;
} process_t;
```

- [ ] Implementar estructura `process_t`
- [ ] Estados de proceso: READY, RUNNING, BLOCKED, ZOMBIE
- [ ] Lista de procesos global
- [ ] Función `process_create(name, entry_point)`
- [ ] Función `process_destroy(pid)`
- [ ] Asignar PID único a cada proceso

### 3.2 Context Switch
- [ ] Función `context_switch(old_process, new_process)` en Assembly
- [ ] Guardar todos los registros del proceso actual
- [ ] Restaurar todos los registros del nuevo proceso
- [ ] Cambiar page directory (CR3)
- [ ] Cambiar stack pointer (RSP)
- [ ] Testing inicial con 2 procesos dummy

### 3.3 Proceso Idle
- [ ] Crear proceso `idle` (PID 0)
- [ ] Loop infinito con `hlt`
- [ ] Menor prioridad posible
- [ ] Siempre ejecutable cuando no hay otro proceso

### 3.4 Testing de Context Switch
- [ ] Crear 2 procesos que imprimen letras diferentes
- [ ] Alternar manualmente entre ellos cada segundo
- [ ] Verificar que mantienen estado independiente
- [ ] Verificar que no corrompen memoria del otro

**Entregable:** Context switching funcional entre procesos.

---

## ⏱️ Bloque 2.4: Scheduler (Planificador)

**Duración estimada:** 3-4 días

**Objetivo:** Implementar scheduler Round-Robin con prioridades.

### 4.1 Scheduler Round-Robin Básico
- [ ] Cola FIFO de procesos READY
- [ ] Función `scheduler_add_process(process)`
- [ ] Función `scheduler_remove_process(pid)`
- [ ] Función `schedule()` - Elige siguiente proceso
- [ ] Quantum de tiempo: 10ms inicial

### 4.2 Integración con Timer
- [ ] Llamar a `schedule()` cada tick del timer
- [ ] Decrementar `time_slice` del proceso actual
- [ ] Hacer context switch cuando `time_slice == 0`
- [ ] Resetear `time_slice` al volver a la cola

### 4.3 Prioridades
- [ ] 4 niveles de prioridad (0=máxima, 3=mínima)
- [ ] Múltiples colas (una por prioridad)
- [ ] Algoritmo: ejecutar cola más prioritaria primero
- [ ] Envejecimiento para evitar starvation

### 4.4 Funciones de Gestión
- [ ] `yield()` - Ceder CPU voluntariamente
- [ ] `sleep(ms)` - Dormir proceso por tiempo
- [ ] `wake_up(pid)` - Despertar proceso
- [ ] Comando shell: `ps` - Listar procesos

### 4.5 Testing del Scheduler
- [ ] Crear 5 procesos con diferentes prioridades
- [ ] Verificar que se respetan prioridades
- [ ] Verificar que todos los procesos avanzan
- [ ] Probar `yield()` y `sleep()`
- [ ] Medir CPU time de cada proceso

**Entregable:** Scheduler funcional con Round-Robin y prioridades.

---

## 🎯 PAUSA 2: REVISIÓN DE MULTITAREA ⏸️

**Duración estimada:** 1-2 días

### Objetivos de la Pausa:
- [ ] Ejecutar 10+ procesos simultáneamente por 10+ minutos
- [ ] Verificar que no hay race conditions
- [ ] Verificar que no hay deadlocks
- [ ] Probar todos los estados de proceso
- [ ] Verificar que prioridades funcionan correctamente
- [ ] Code review del scheduler y context switch
- [ ] Documentar algoritmos de scheduling
- [ ] Optimizar performance si es necesario

**Entregable:** Sistema de multitarea estable y eficiente.

---

## 🧵 Bloque 2.5: Syscalls y User Mode

**Duración estimada:** 4-5 días

**Objetivo:** Implementar syscalls y ejecutar código en user mode (ring 3).

### 5.1 Infraestructura de Syscalls
- [ ] Configurar SYSCALL/SYSRET (MSRs)
- [ ] Handler de syscalls en Assembly
- [ ] Tabla de syscalls (array de función pointers)
- [ ] Pasar argumentos en registros (RDI, RSI, RDX, R10, R8, R9)
- [ ] Retornar valores en RAX

### 5.2 Syscalls Básicas (10 syscalls iniciales)
- [ ] `sys_exit(code)` - Terminar proceso
- [ ] `sys_write(fd, buf, len)` - Escribir en consola
- [ ] `sys_read(fd, buf, len)` - Leer de teclado
- [ ] `sys_yield()` - Ceder CPU
- [ ] `sys_sleep(ms)` - Dormir proceso
- [ ] `sys_getpid()` - Obtener PID
- [ ] `sys_fork()` - Crear proceso hijo (básico)
- [ ] `sys_malloc(size)` - Allocar memoria user
- [ ] `sys_free(ptr)` - Liberar memoria user
- [ ] `sys_get_ticks()` - Obtener tiempo del sistema

### 5.3 User Mode Setup
- [ ] GDT con segmentos de user mode (ring 3)
- [ ] Crear stack de usuario para procesos
- [ ] Función `enter_usermode(entry_point)`
- [ ] Separar kernel stack y user stack

### 5.4 Proceso de Usuario Simple
Crear primer programa en user mode:

```c
void user_program() {
    while(1) {
        sys_write(1, "Hello from user!\n", 17);
        sys_sleep(1000);
    }
}
```

- [ ] Compilar y linkar código de usuario separado
- [ ] Cargar en memoria de usuario
- [ ] Ejecutar en ring 3
- [ ] Verificar que syscalls funcionan

### 5.5 Testing de User Mode
- [ ] Verificar que user mode no puede acceder kernel memory
- [ ] Provocar General Protection Fault intencional
- [ ] Verificar que syscalls funcionan correctamente
- [ ] Crear múltiples procesos de usuario
- [ ] Comando shell: `exec [program]` - Ejecutar programa

**Entregable:** Syscalls funcionales y código ejecutándose en user mode.

---

## 🎯 PAUSA 3: REVISIÓN COMPLETA DEL SISTEMA ⏸️

**Duración estimada:** 2-3 días

### Objetivos Finales de Fase 2:

#### 3.1 Testing Integrado Completo
- [ ] Sistema corriendo 1+ hora sin crashes
- [ ] 10+ procesos simultáneos funcionando
- [ ] Todos los syscalls probados exhaustivamente
- [ ] Memory leaks verificados (usar comandos de debug)
- [ ] Performance aceptable (responsive al usuario)

#### 3.2 Comandos Shell Adicionales
- [ ] `ps` - Lista de procesos con detalles
- [ ] `kill [pid]` - Terminar proceso
- [ ] `top` - Monitor de procesos (simple)
- [ ] `meminfo` - Info detallada de memoria
- [ ] `test` - Suite completa de tests del sistema

#### 3.3 Documentación Completa
- [ ] README.md actualizado con nuevas features
- [ ] Documentar todas las syscalls
- [ ] Diagramas de:
  - Flujo de context switch
  - Estructura de memoria virtual
  - Scheduler algorithm
  - Syscall handling
- [ ] Guía de programación de user programs
- [ ] Lista de limitaciones conocidas

#### 3.4 Code Quality Final
- [ ] Sin warnings del compilador
- [ ] Todos los archivos comentados
- [ ] Naming consistente
- [ ] Eliminar código de debug innecesario
- [ ] Añadir asserts donde corresponda
- [ ] Verificar que todos los errores se manejan correctamente

#### 3.5 Preparación para Fase 3
- [ ] Lista de TODOs para Fase 3:
  - Sistema de archivos (VFS + ramdisk)
  - Drivers adicionales (RTC, serial port)
  - IPC (Inter-Process Communication)
  - Señales básicas
  - ELF loader para ejecutables
- [ ] Identificar cuellos de botella de performance
- [ ] Planear mejoras arquitecturales necesarias

**Entregable:** Sistema operativo funcional con multitarea real, memoria virtual completa, y capacidad de ejecutar código de usuario mediante syscalls.

---

## 📊 Resumen de la Fase 2

### ✅ Al completar esta fase tendrá:
- Gestión completa de memoria física y virtual
- Heap allocator con malloc/free
- Multitarea preemptiva con scheduler
- Context switching robusto
- User mode (ring 3) funcional
- 10 syscalls básicas
- Múltiples procesos ejecutándose simultáneamente
- Sistema estable y bien documentado

---
