// shell.h - Declaraciones del shell mejorado
#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

// Estructura para comandos del shell
typedef struct {
    char* name;
    char* description;
    void (*function)(int argc, char** argv);
} shell_command_t;

// Constantes del shell
#define SHELL_BUFFER_SIZE 256
#define SHELL_MAX_ARGS 8
#define SHELL_HISTORY_SIZE 10
#define SHELL_PROMPT "os> "

// Funciones del shell
void shell_init(void);
void shell_run(void);
void shell_execute_command(const char* input);
char* shell_readline(void);
void shell_parse_command(const char* input, int* argc, char* argv[]);
void shell_add_to_history(const char* command);
void shell_show_history(void);

// Comandos del shell
void shell_cmd_help(int argc, char** argv);
void shell_cmd_clear(int argc, char** argv);
void shell_cmd_echo(int argc, char** argv);
void shell_cmd_uptime(int argc, char** argv);
void shell_cmd_halt(int argc, char** argv);
void shell_cmd_history(int argc, char** argv);
void shell_cmd_reboot(int argc, char** argv);
void shell_cmd_meminfo(int argc, char** argv);
void shell_cmd_heapinfo(int argc, char** argv);

#endif