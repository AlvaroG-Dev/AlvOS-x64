# 📋 PLANIFICACIÓN FASE 2: EXPANSIÓN DEL KERNEL

## 🎯 Objetivo de la Fase 2
Transformar el kernel básico en un sistema más avanzado con gestión de memoria virtual, sistema de archivos, procesos básicos y llamadas al sistema.

**Duración estimada:** 6-8 semanas

---

## 🧠 Bloque 2.1: Virtual Memory Manager (VMM)
**Duración estimada:** 2 semanas

### 2.1.1 Paginación Avanzada
- [ ] Implementar higher-half kernel (kernel en 0xFFFFFFFF80000000)
- [ ] Mapeo dinámico de páginas (4KB) además de 2MB
- [ ] Page fault handler avanzado con información detallada
- [ ] Copy-on-write para futuras optimizaciones

### 2.1.2 Memory Manager Mejorado
- [ ] Implementar `kfree()` para el heap
- [ ] Cambiar de bump allocator a buddy allocator o slab allocator
- [ ] Heap expandible dinámicamente
- [ ] Memory pools para estructuras comunes

### 2.1.3 Funcionalidades Avanzadas
- [ ] Demand paging (carga bajo demanda)
- [ ] Memory protection (R/W/X bits)
- [ ] Guardar/restaurar tablas de páginas
- [ ] Comando `vminfo` para información de memoria virtual

---

## 🎯 PAUSA 1: REVISIÓN MEMORIA VIRTUAL ⏸️
**Duración estimada:** 2-3 días

### Objetivos
- [ ] Verificar que higher-half kernel funciona correctamente
- [ ] Probar page fault handler con diferentes escenarios
- [ ] Validar que `kfree()` no causa corrupción
- [ ] Benchmark de performance del nuevo allocator
- [ ] Comprobar integridad de tablas de páginas

### Testing Específico
- [ ] Provocar page faults intencionales
- [ ] Allocar/liberar memoria intensivamente
- [ ] Verificar protección de memoria (intentar escribir en páginas RO)
- [ ] Test de estrés con 1000+ allocaciones/liberaciones

---

## 💾 Bloque 2.2: Sistema de Archivos Básico
**Duración estimada:** 2 semanas

### 2.2.1 Virtual File System (VFS)
- [ ] Estructuras VFS: `vfs_node_t`, `vfs_operations_t`
- [ ] Operaciones básicas: open, read, write, close, seek
- [ ] Sistema de montaje de filesystems
- [ ] Path resolution ("/dev/console")

### 2.2.2 Initrd/Initramfs
- [ ] Cargar initrd desde Multiboot2 modules
- [ ] Filesystem en memoria (tar o formato simple)
- [ ] Navegación básica de directorios
- [ ] Carga de archivos de configuración

### 2.2.3 Filesystems Implementados
- [ ] **devfs**: Sistema de archivos de dispositivos  
  - [ ] /dev/console, /dev/null, /dev/zero
- [ ] **procfs**: Sistema de archivos de procesos  
  - [ ] /proc/meminfo, /proc/version, /proc/uptime
- [ ] **tmpfs**: Sistema de archivos temporal

### 2.2.4 Comandos de Archivos
- [ ] `ls` — Listar directorios
- [ ] `cat` — Mostrar contenido de archivos
- [ ] `pwd` — Directorio actual
- [ ] `cd` — Cambiar directorio

---

## 🎯 PAUSA 2: REVISIÓN FILESYSTEM ⏸️
**Duración estimada:** 2-3 días

### Objetivos
- [ ] Verificar que VFS maneja correctamente múltiples filesystems
- [ ] Probar navegación de directorios en initrd
- [ ] Validar operaciones de archivos (leer, escribir, buscar)
- [ ] Comprobar integridad de datos en filesystems
- [ ] Test de permisos y errores

### Testing Específico
- [ ] Crear y leer archivos en tmpfs
- [ ] Navegar árbol de directorios complejo
- [ ] Probar dispositivos en devfs
- [ ] Verificar información en procfs
- [ ] Test de archivos muy grandes (>1MB)

---

## 🔄 Bloque 2.3: Gestión de Procesos Básica
**Duración estimada:** 1.5 semanas

### 2.3.1 Estructuras de Proceso
- [ ] `process_t` — Estructura de proceso
- [ ] PID, estado, prioridad
- [ ] Contexto (registros, stack)
- [ ] Tabla de páginas
- [ ] File descriptors
- [ ] PCB (Process Control Block)

### 2.3.2 Context Switching
- [ ] Guardar/restaurar contexto completo
- [ ] Switch de tablas de páginas (CR3)
- [ ] Manejo de TLB flushes

### 2.3.3 Scheduler Simple
- [ ] Scheduler round-robin
- [ ] Colas de procesos listos, bloqueados, terminados
- [ ] Timer tick para preemption
- [ ] Cambio de proceso cada 10-100ms

### 2.3.4 Comandos de Procesos
- [ ] `ps` — Listar procesos
- [ ] `kill` — Terminar proceso
- [ ] `sleep` — Dormir proceso

---

## 🎯 PAUSA 3: REVISIÓN PROCESOS ⏸️
**Duración estimada:** 2 días

### Objetivos
- [ ] Verificar context switching estable
- [ ] Probar scheduler con múltiples procesos
- [ ] Validar aislamiento entre procesos
- [ ] Comprobar manejo de estados de proceso

### Testing Específico
- [ ] Crear 10+ procesos simultáneos
- [ ] Probar cambio de contexto intensivo
- [ ] Verificar que procesos terminados se limpian
- [ ] Test de prioridades y scheduling

---

## 📞 Bloque 2.4: Sistema de Llamadas al Sistema
**Duración estimada:** 1.5 semanas

### 2.4.1 Mecanismo de Syscalls
- [ ] Interrupción syscall (instrucción SYSCALL)
- [ ] MSRs: STAR, LSTAR, SFMASK
- [ ] Cambio automático ring 3 → 0 (kernel mode)
- [ ] Handler de syscalls en C

### 2.4.2 Syscalls Esenciales
- [ ] `sys_exit` — Terminar proceso
- [ ] `sys_read` — Leer de file descriptor
- [ ] `sys_write` — Escribir a file descriptor
- [ ] `sys_open` — Abrir archivo
- [ ] `sys_close` — Cerrar archivo
- [ ] `sys_brk` — Cambiar tamaño de heap
- [ ] `sys_getpid` — Obtener PID

### 2.4.3 User Mode Support
- [ ] Configurar segmentos para user mode (ring 3)
- [ ] Stack separado para user mode
- [ ] Protección de memoria user/kernel

---

## 🎯 PAUSA 4: REVISIÓN SYSCALLS ⏸️
**Duración estimada:** 2 días

### Objetivos
- [ ] Verificar transición segura user → kernel
- [ ] Probar todas las syscalls implementadas
- [ ] Validar protección entre espacios de memoria
- [ ] Comprobar manejo de errores en syscalls

### Testing Específico
- [ ] Provocar syscalls con parámetros inválidos
- [ ] Test de permisos entre user/kernel
- [ ] Verificar que user mode no puede acceder a kernel
- [ ] Stress test de syscalls concurrentes

---

## 🛠️ Bloque 2.5: Drivers y Dispositivos
**Duración estimada:** 1 semana

### 2.5.1 Sistema de Drivers
- [ ] Estructura `device_t` para dispositivos
- [ ] Registro de drivers
- [ ] Major/minor numbers
- [ ] Device nodes en devfs

### 2.5.2 Console Driver Mejorado
- [ ] Soporte para múltiples consoles virtuales
- [ ] Scrollback buffer
- [ ] Control de terminal (VT100 básico)
- [ ] Echo de entrada/salida

### 2.5.3 RTC (Real Time Clock)
- [ ] Leer fecha/hora actual
- [ ] Comando `date`
- [ ] Alarmas básicas

---

## 🧪 Bloque 2.6: Testing y Debugging Avanzado
**Duración estimada:** 1 semana

### 2.6.1 Kernel Debugging
- [ ] Backtraces en modo kernel
- [ ] Información de procesos en page faults
- [ ] Memory leak detection básico
- [ ] Kernel logs con niveles (INFO, WARN, ERROR)

### 2.6.2 Testing de Fase 2
- [ ] Tests de memoria virtual
- [ ] Tests de filesystem
- [ ] Tests de procesos y scheduling
- [ ] Tests de syscalls
- [ ] Stress tests de memoria

### 2.6.3 Comandos de Debug
- [ ] `crash` — Provocar kernel panic controlado
- [ ] `memtest` — Test avanzado de memoria
- [ ] `trace` — Seguimiento de llamadas
- [ ] `stats` — Estadísticas del sistema

---

## 🎯 PAUSA FINAL: INTEGRACIÓN COMPLETA ⏸️
**Duración estimada:** 3-4 días

### Objetivos Finales
- [ ] Integración completa de todos los subsistemas
- [ ] Estabilidad con todas las funcionalidades activas
- [ ] Performance aceptable en todas las operaciones
- [ ] Recovery de errores en todos los niveles

### Testing Integral
- [ ] Ejecutar procesos que usen filesystem y syscalls
- [ ] Probar memoria virtual con múltiples procesos
- [ ] Verificar aislamiento entre user y kernel space
- [ ] Stress test del sistema completo

### Métricas de Calidad
- [ ] 0 memory leaks detectados
- [ ] 0 crashes en 24 horas de ejecución
- [ ] Todas las syscalls responden correctamente
- [ ] Filesystem mantiene integridad de datos

---

## 📊 CRITERIOS DE ÉXITO FASE 2

### ✅ Funcionalidades Clave Completadas
- [ ] **Memoria Virtual:** Kernel en higher-half, page faults manejados
- [ ] **Filesystem:** Navegación básica con initrd, VFS funcionando
- [ ] **Procesos:** Múltiples procesos ejecutándose concurrentemente
- [ ] **Syscalls:** Aplicaciones básicas pueden llamar al kernel
- [ ] **User Mode:** Ejecución segura en ring 3

### ✅ Estabilidad Garantizada
- [ ] No memory leaks en PMM/VMM
- [ ] Context switching sin corrupción
- [ ] Syscalls manejando errores correctamente
- [ ] Filesystem sin corrupción de datos

### ✅ Testing Completo
- [ ] Suite de tests expandida cubriendo nuevas funcionalidades
- [ ] Tests de estrés con múltiples procesos
- [ ] Recovery de errores (page faults, syscalls inválidos)
- [ ] Benchmarks básicos de performance

---

## 🚀 PREPARACIÓN PARA FASE 3

### Cimientos Establecidos
- [ ] **Memoria Virtual** → Permitirá procesos aislados
- [ ] **Sistema de Archivos** → Base para almacenamiento persistente
- [ ] **Procesos y Syscalls** → Fundamentos para multitarea real
- [ ] **User Mode** → Seguridad y aislamiento

### Próximos Pasos (Fase 3)
- [ ] Multitarea preemptiva avanzada
- [ ] Driver ATA/IDE para discos reales
- [ ] Filesystem persistente (FAT32 o ext2)
- [ ] Sistema de módulos cargables
- [ ] Stack TCP/IP básico
- [ ] Interfaz de usuario gráfica básica

---

## 📅 CALENDARIO ESTIMADO FASE 2

### Semanas 1-2: Memoria Virtual
- [ ] Higher-half kernel funcionando
- [ ] Page fault handler con información útil
- [ ] `kfree()` implementado y probado

### Semanas 3-4: Sistema de Archivos
- [ ] VFS con operaciones básicas
- [ ] Initrd cargando y navegable
- [ ] Comandos `ls`/`cat` funcionando

### Semanas 5-6: Procesos y Syscalls
- [ ] Múltiples procesos ejecutándose
- [ ] Context switching estable
- [ ] Syscalls básicas respondiendo

### Semanas 7-8: Integración y Testing
- [ ] Todo integrado y estable
- [ ] Tests completos pasando
- [ ] Documentación actualizada