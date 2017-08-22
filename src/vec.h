#ifndef VEC_H
#define VEC_H

#include <stddef.h>

typedef struct {
    size_t size;
    size_t elem_size;
    char data[];
} * vec_t;

vec_t vec_create(size_t n, size_t elemsize);
void vec_destroy(vec_t vec);

void vec_set(vec_t* vec_ref, size_t i, const void* elem);
int vec_get(const vec_t vec, size_t i, void* dst);

void _vec_destroy(vec_t* vec_ref);
#define auto_vec_t vec_t __attribute__((cleanup(_vec_destroy)))

#define vec_size(vec) (vec->size / vec->elem_size)

#endif