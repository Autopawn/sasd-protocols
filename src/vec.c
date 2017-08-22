#include "vec.h"
#include <stdlib.h>
#include <string.h>

vec_t __vec_grow(vec_t* vec_ref)
{
    vec_t vec = *vec_ref;
    size_t prev_size = vec->size;
    size_t count = vec_size(vec);
    count += count / 2;
    vec->size = count * vec->elem_size;
    vec = *vec_ref = realloc(vec, sizeof(*vec) + vec->size);

    if (vec->zero_fill) {
        memset(vec->data + prev_size, 0, vec->size - prev_size);
    }

    return vec;
}

vec_t vec_create(size_t n, size_t elemsize, int zero_fill)
{
    vec_t vec = malloc(sizeof(*vec) + n * elemsize);
    vec->size = n * elemsize;
    vec->elem_size = elemsize;
    vec->zero_fill = zero_fill;

    if (zero_fill) {
        memset(vec->data, 0, vec->size);
    }

    return vec;
}

void vec_destroy(vec_t vec)
{
    free(vec);
}

void vec_set(vec_t* vec_ref, size_t i, const void* elem)
{
    vec_t vec = *vec_ref;
    if (i >= vec_size(vec)) {
        vec = __vec_grow(vec_ref);
    }
    memcpy(vec->data + i * vec->elem_size, elem, vec->elem_size);
}

int vec_get(const vec_t vec, size_t i, void* dst)
{
    if (i >= vec_size(vec)) {
        return -1;
    }
    memcpy(dst, vec->data + i * vec->elem_size, vec->elem_size);
    return 0;
}

void _vec_destroy(vec_t* vec_ref)
{
    if (vec_ref != NULL) {
        vec_destroy(*vec_ref);
    }
}