#include "bvec.h"

#include <stdlib.h>
#include <string.h>

bvec_t bv_create(size_t size)
{
    bvec_t vec = malloc(sizeof(*vec) + size);
    vec->size = size;
    vec->wptr = 0;
    return vec;
}

void bv_append(bvec_t* vec_ref, const void* data, size_t size)
{
    bvec_t vec = *vec_ref;
    if (vec->size - vec->wptr < size) {
        vec->size += vec->size / 2;
        vec = *vec_ref = realloc(vec,
            sizeof(*vec) + vec->size);
    }
    memcpy(vec->data + vec->wptr, data, size);
    vec->wptr += size;
}

void bv_destroy(bvec_t vec)
{
    free(vec);
}

void _bv_destroy(bvec_t* vec_ref)
{
    if (vec_ref != NULL) {
        bv_destroy(*vec_ref);
    }
}