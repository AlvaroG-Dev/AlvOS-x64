// string.h - Declaraciones de funciones de string
#ifndef STRING_H
#define STRING_H

#include <stddef.h>

// Funciones de string
size_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
char* strchr(const char* str, int c);

// Funciones de memoria
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* ptr, int value, size_t n);
int memcmp(const void* ptr1, const void* ptr2, size_t n);
void* memmove(void* dest, const void* src, size_t n);

#endif