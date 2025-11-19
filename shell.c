// shell.c - Shell mejorado con historial y mejores mensajes
#include "shell.h"
#include "terminal.h"
#include "keyboard.h"
#include "timer.h"
#include "string.h"
#include "panic.h"
#include "memory.h"

// Buffer para entrada del usuario
static char shell_buffer[SHELL_BUFFER_SIZE];
static size_t shell_buffer_pos = 0;

// Buffer temporal para parseo
static char parse_buffer[SHELL_BUFFER_SIZE];

// Historial de comandos
static char shell_history[SHELL_HISTORY_SIZE][SHELL_BUFFER_SIZE];
static int history_count = 0;
static int history_current = -1;

// Lista de comandos disponibles (MEJORADA CON DESCRIPCIONES DETALLADAS)
static shell_command_t shell_commands[] = {
    {"help",    "Muestra información sobre todos los comandos disponibles", shell_cmd_help},
    {"clear",   "Limpia completamente la pantalla del terminal", shell_cmd_clear},
    {"echo",    "Repite el texto ingresado como argumento", shell_cmd_echo},
    {"uptime",  "Muestra el tiempo que el sistema ha estado activo", shell_cmd_uptime},
    {"halt",    "Detiene el sistema de forma segura (apagar)", shell_cmd_halt},
    {"history", "Muestra el historial de comandos recientes", shell_cmd_history},
    {"reboot",  "Reinicia el sistema", shell_cmd_reboot},
    {"meminfo", "Muestra información detallada de la memoria", shell_cmd_meminfo}, // NUEVO
    {NULL, NULL, NULL} // Marcador de fin
};

// ========== FUNCIONES AUXILIARES ==========

// Agregar comando al historial
void shell_add_to_history(const char* command) {
    // No agregar comandos vacíos o duplicados consecutivos
    if (strlen(command) == 0) return;
    if (history_count > 0 && strcmp(shell_history[history_count-1], command) == 0) return;
    
    // Si el historial está lleno, desplazar comandos viejos
    if (history_count >= SHELL_HISTORY_SIZE) {
        for (int i = 0; i < SHELL_HISTORY_SIZE - 1; i++) {
            strcpy(shell_history[i], shell_history[i+1]);
        }
        history_count--;
    }
    
    // Agregar nuevo comando
    strcpy(shell_history[history_count], command);
    history_count++;
    history_current = history_count; // Resetear navegación
}

// Mostrar historial de comandos
void shell_show_history(void) {
    if (history_count == 0) {
        terminal_writestring("No hay comandos en el historial.\n");
        return;
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("Historial de comandos (más reciente al final):\n");
    terminal_writestring("==============================================\n");
    
    for (int i = 0; i < history_count; i++) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
        terminal_writestring("  ");
        print_dec(i + 1);
        terminal_writestring(". ");
        
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
        terminal_writestring(shell_history[i]);
        terminal_putchar('\n');
    }
}

// Inicializar el shell
void shell_init(void) {
    shell_buffer_pos = 0;
    shell_buffer[0] = '\0';
    history_count = 0;
    history_current = -1;
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Shell inicializado.\n");
    terminal_writestring("Escribe 'help' para ver comandos disponibles.\n\n");
}

// Mostrar prompt mejorado
static void shell_show_prompt(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("\n-[");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("AlvOS");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("]\n");
    terminal_writestring("$ ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

// Leer una línea de entrada (MEJORADO)
char* shell_readline(void) {
    shell_buffer_pos = 0;
    shell_buffer[0] = '\0';
    
    shell_show_prompt();
    
    while (1) {
        char c = keyboard_getchar();
        
        // Enter - ejecutar comando
        if (c == '\n') {
            terminal_putchar('\n');
            shell_buffer[shell_buffer_pos] = '\0';
            
            // Agregar al historial si no está vacío
            if (strlen(shell_buffer) > 0) {
                shell_add_to_history(shell_buffer);
            }
            
            return shell_buffer;
        }
        
        // Backspace
        else if (c == '\b') {
            if (shell_buffer_pos > 0) {
                shell_buffer_pos--;
                shell_buffer[shell_buffer_pos] = '\0';
                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
            }
        }
        
        // Carácter normal (si hay espacio)
        else if (shell_buffer_pos < SHELL_BUFFER_SIZE - 1 && c >= 32 && c <= 126) {
            shell_buffer[shell_buffer_pos++] = c;
            shell_buffer[shell_buffer_pos] = '\0';
            terminal_putchar(c);
        }
        
        // Ignorar otros caracteres
    }
}

// Parsear comando y argumentos (MEJORADO)
void shell_parse_command(const char* input, int* argc, char* argv[]) {
    *argc = 0;
    
    // Copiar input a buffer temporal para modificar
    strcpy(parse_buffer, input);
    char* p = parse_buffer;
    
    // Saltar espacios iniciales
    while (*p == ' ') p++;
    
    while (*p != '\0' && *argc < SHELL_MAX_ARGS) {
        if (*p == '"') {
            // Argumento entre comillas
            p++;
            argv[*argc] = p;
            
            // Buscar fin de comilla
            while (*p != '\0' && *p != '"') p++;
            
            if (*p == '"') {
                *p = '\0';
                p++;
            }
        } else {
            // Argumento normal
            argv[*argc] = p;
            
            // Buscar fin del argumento
            while (*p != '\0' && *p != ' ') p++;
            
            if (*p == ' ') {
                *p = '\0';
                p++;
            }
        }
        
        // Solo agregar argumento si no está vacío
        if (strlen(argv[*argc]) > 0) {
            (*argc)++;
        }
        
        // Saltar espacios
        while (*p == ' ') p++;
    }
}

// Ejecutar comando (MEJORADO CON MEJORES MENSAJES DE ERROR)
void shell_execute_command(const char* input) {
    // Verificar si es línea vacía
    const char* p = input;
    while (*p == ' ') p++;
    if (*p == '\0') return;
    
    int argc = 0;
    char* argv[SHELL_MAX_ARGS];
    
    shell_parse_command(input, &argc, argv);
    
    if (argc == 0) return;
    
    // Buscar comando
    shell_command_t* cmd = shell_commands;
    while (cmd->name != NULL) {
        if (strcmp(argv[0], cmd->name) == 0) {
            cmd->function(argc, argv);
            return;
        }
        cmd++;
    }
    
    // Comando no encontrado - MEJOR MENSAJE DE ERROR
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    terminal_writestring("Error: El comando '");
    terminal_writestring(argv[0]);
    terminal_writestring("' no existe.\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("Sugerencia: Escribe 'help' para ver todos los comandos disponibles.\n");
    
    // Mostrar comandos similares (búsqueda aproximada)
    terminal_writestring("Comandos similares: ");
    int suggestions = 0;
    cmd = shell_commands;
    while (cmd->name != NULL) {
        // Búsqueda simple por coincidencia de primeros caracteres
        if (strncmp(argv[0], cmd->name, strlen(argv[0])) == 0) {
            if (suggestions > 0) terminal_writestring(", ");
            terminal_writestring(cmd->name);
            suggestions++;
        }
        cmd++;
    }
    
    if (suggestions == 0) {
        terminal_writestring("ninguno encontrado");
    }
    terminal_putchar('\n');
}

// Loop principal del shell
void shell_run(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("\nSistema listo. Shell interactivo iniciado.\n");
    terminal_writestring("Escribe 'help' para comenzar.\n\n");
    
    while (1) {
        char* input = shell_readline();
        shell_execute_command(input);
    }
}

// ========== IMPLEMENTACIÓN DE COMANDOS MEJORADOS ==========

/**
 * COMANDO: help
 * DESCRIPCIÓN: Muestra información detallada sobre todos los comandos disponibles
 * USO: help [comando]
 */
void shell_cmd_help(int argc, char** argv) {
    if (argc == 1) {
        // Mostrar todos los comandos
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("COMANDOS DISPONIBLES\n");
        terminal_writestring("=======================\n");
        terminal_writestring("Escribe 'help <comando>' para información detallada.\n\n");
        
        shell_command_t* cmd = shell_commands;
        while (cmd->name != NULL) {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
            terminal_writestring("  ");
            terminal_writestring(cmd->name);
            
            // Calcular espacios para alineación
            int name_len = strlen(cmd->name);
            int spaces = 12 - name_len;
            if (spaces < 1) spaces = 1;
            
            for (int i = 0; i < spaces; i++) {
                terminal_putchar(' ');
            }
            
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
            terminal_writestring("- ");
            terminal_writestring(cmd->description);
            terminal_putchar('\n');
            
            cmd++;
        }
    } else if (argc == 2) {
        // Mostrar ayuda específica para un comando
        shell_command_t* cmd = shell_commands;
        while (cmd->name != NULL) {
            if (strcmp(argv[1], cmd->name) == 0) {
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
                terminal_writestring("AYUDA DETALLADA: ");
                terminal_writestring(cmd->name);
                terminal_writestring("\n");
                terminal_writestring("===================================\n");
                
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                terminal_writestring("Descripción: ");
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
                terminal_writestring(cmd->description);
                terminal_writestring("\n");
                
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                terminal_writestring("Uso: ");
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
                terminal_writestring(cmd->name);
                
                // Ejemplos específicos por comando
                if (strcmp(cmd->name, "echo") == 0) {
                    terminal_writestring(" <texto>\n");
                    terminal_writestring("   Ejemplo: echo Hola Mundo\n");
                } else if (strcmp(cmd->name, "uptime") == 0) {
                    terminal_writestring("\n");
                    terminal_writestring("   Muestra el tiempo del sistema en segundos y formato HH:MM:SS\n");
                }
                
                return;
            }
            cmd++;
        }
        
        // Comando no encontrado para ayuda específica
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("No se encontró ayuda para el comando '");
        terminal_writestring(argv[1]);
        terminal_writestring("'\n");
    } else {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("Uso incorrecto. Formato: help [comando]\n");
    }
}

/**
 * COMANDO: clear
 * DESCRIPCIÓN: Limpia completamente la pantalla del terminal
 * USO: clear
 */
void shell_cmd_clear(int argc, char** argv) {
    (void)argc; (void)argv;
    terminal_initialize();
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring("Pantalla limpiada.\n\n");
}

/**
 * COMANDO: echo
 * DESCRIPCIÓN: Repite el texto ingresado como argumento
 * USO: echo <texto>
 */
void shell_cmd_echo(int argc, char** argv) {
    if (argc < 2) {
        terminal_writestring("\n");  // Solo echo sin argumentos = nueva línea
        return;
    }
    
    for (int i = 1; i < argc; i++) {
        terminal_writestring(argv[i]);
        if (i < argc - 1) {
            terminal_putchar(' ');
        }
    }
    terminal_putchar('\n');
}

/**
 * COMANDO: uptime
 * DESCRIPCIÓN: Muestra el tiempo que el sistema ha estado activo
 * USO: uptime
 */
void shell_cmd_uptime(int argc, char** argv) {
    (void)argc; (void)argv;
    
    uint64_t ticks = timer_get_ticks();
    uint64_t seconds = ticks / 100;  // 100Hz = 100 ticks por segundo
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("TIEMPO DE ACTIVIDAD DEL SISTEMA\n");
    terminal_writestring("==================================\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Segundos: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    print_dec(seconds);
    terminal_writestring(" segundos\n");
    
    // Mostrar también en formato HH:MM:SS
    uint64_t hours = seconds / 3600;
    uint64_t minutes = (seconds % 3600) / 60;
    uint64_t secs = seconds % 60;
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Formateado: ");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    if (hours < 10) terminal_putchar('0');
    print_dec(hours);
    terminal_putchar(':');
    if (minutes < 10) terminal_putchar('0');
    print_dec(minutes);
    terminal_putchar(':');
    if (secs < 10) terminal_putchar('0');
    print_dec(secs);
    terminal_putchar('\n');
}

/**
 * COMANDO: halt
 * DESCRIPCIÓN: Detiene el sistema de forma segura
 * USO: halt
 */
void shell_cmd_halt(int argc, char** argv) {
    (void)argc; (void)argv;
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    terminal_writestring("\nDETENIENDO EL SISTEMA\n");
    terminal_writestring("=======================\n");
    terminal_writestring("El sistema se está apagando de forma segura...\n");
    terminal_writestring("Puedes cerrar QEMU o reiniciar la máquina virtual.\n\n");
    
    // Detener el sistema
    while (1) {
        __asm__ volatile ("cli");
        __asm__ volatile ("hlt");
    }
}

/**
 * COMANDO: history
 * DESCRIPCIÓN: Muestra el historial de comandos recientes
 * USO: history
 */
void shell_cmd_history(int argc, char** argv) {
    (void)argc; (void)argv;
    shell_show_history();
}

/**
 * COMANDO: reboot
 * DESCRIPCIÓN: Reinicia el sistema
 * USO: reboot
 */
void shell_cmd_reboot(int argc, char** argv) {
    (void)argc; (void)argv;
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("\nREINICIANDO EL SISTEMA\n");
    terminal_writestring("========================\n");
    terminal_writestring("El sistema se reiniciara...\n");
    
    // Simular reinicio (en un sistema real sería más complejo)
    for (int i = 3; i > 0; i--) {
        terminal_writestring("Reiniciando en ");
        print_dec(i);
        terminal_writestring("...\n");
        timer_wait(1000); // Esperar 1 segundo
    }
    
    terminal_writestring("Reiniciando ahora...\n");
    
    // En un sistema real, aquí se haría un reset del CPU
    // Por ahora, simplemente reiniciamos el shell
    terminal_initialize();
    shell_init();
    shell_run();
}

/**
 * COMANDO: meminfo
 * DESCRIPCIÓN: Muestra información detallada de la memoria del sistema
 * USO: meminfo
 */
void shell_cmd_meminfo(int argc, char** argv) {
    (void)argc; (void)argv;
    cmd_meminfo(argc, argv);  // Llama a la función en memory.c
}