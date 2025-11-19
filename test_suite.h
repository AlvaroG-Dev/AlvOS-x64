// test_suite.h - Declaraciones de la suite de pruebas
#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <stdint.h>
#include "exception_test.h"

// Ejecutar la suite completa de tests
void run_test_suite(void);

// Tests individuales
void test_timer_basic(void);
void test_timer_overflow(void);
void test_keyboard_lowercase(void);
void test_keyboard_shift(void);
void test_keyboard_numbers(void);
void test_keyboard_backspace(void);
void test_terminal_colors(void);
void test_terminal_scroll(void);


// Utilidades
void test_print_pass(const char* test_name);
void test_print_fail(const char* test_name);
void test_print_info(const char* message);
void test_print_warn(const char* message);
void test_print_summary(void);

#endif