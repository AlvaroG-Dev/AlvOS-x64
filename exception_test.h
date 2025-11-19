#ifndef EXCEPTION_TEST_H
#define EXCEPTION_TEST_H

#include <stdint.h>

// Pruebas de excepciones
void run_exception_tests(void);
void run_quick_exception_test(uint8_t test_num);
void test_divide_by_zero(void);
void test_invalid_opcode(void);
void test_page_fault(void);
void test_general_protection(void);
void test_stack_fault(void);
void test_stack_fault_recursive(void);

#endif