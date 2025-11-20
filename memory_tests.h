// memory_tests.h - Prototipos para los tests de memoria (kmalloc y PMM)
#ifndef MEMORY_TESTS_H
#define MEMORY_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Prototipos de los tests definidos en memory_tests.c */
void test_kmalloc_small_blocks(void);
void test_kmalloc_alignment(void);
void test_kmalloc_out_of_memory(void);

void test_pmm_alloc_free(void);
void test_pmm_no_overlap(void);

/* Ejecuta todos los tests de memoria */
void run_memory_tests(void);

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_TESTS_H */
