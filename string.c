// string.c - Implementación de funciones de string estándar
#include "string.h"

// Calcular longitud de string
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

// Comparar dos strings
int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

// Comparar n caracteres de dos strings
int strncmp(const char* str1, const char* str2, size_t n) {
    while (n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0)
        return 0;
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

// Copiar string
char* strcpy(char* dest, const char* src) {
    char* original_dest = dest;
    while ((*dest++ = *src++))
        ;
    return original_dest;
}

// Copiar n caracteres de string
char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
    return dest;
}

// Concatenar strings
char* strcat(char* dest, const char* src) {
    char* original_dest = dest;
    while (*dest)
        dest++;
    while ((*dest++ = *src++))
        ;
    return original_dest;
}

// Buscar carácter en string
char* strchr(const char* str, int c) {
    while (*str != (char)c) {
        if (!*str++)
            return NULL;
    }
    return (char*)str;
}

// Copiar memoria
void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (size_t i = 0; i < n; i++)
        d[i] = s[i];
    return dest;
}

// Establecer memoria a un valor
void* memset(void* ptr, int value, size_t n) {
    unsigned char* p = (unsigned char*)ptr;
    for (size_t i = 0; i < n; i++)
        p[i] = (unsigned char)value;
    return ptr;
}

// Comparar memoria
int memcmp(const void* ptr1, const void* ptr2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i])
            return p1[i] - p2[i];
    }
    return 0;
}

// Mover memoria (maneja overlaps)
void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d < s) {
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    } else {
        for (size_t i = n; i > 0; i--)
            d[i-1] = s[i-1];
    }
    return dest;
}