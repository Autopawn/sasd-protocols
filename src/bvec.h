#ifndef BVEC_H
#define BVEC_H

#include <stddef.h>

typedef struct {
    size_t wptr;
    size_t size;
    char data[];

} * bvec_t;

/*
 * Create new byte vector sith @size bytes
 */
bvec_t bv_create(size_t size);

/*
 * Append data to vector
 */
void bv_append(bvec_t* vec_ref, const void* data, size_t size);

/*
 * Destroy byte vector
 */
void bv_destroy(bvec_t vec);

// Get vector data (w/r)
#define bv_data(vec) \
    (vec->data)
// Reset write posiition (equivalent to bv_size(vec) = 0)
#define bv_reset(vec) \
    vec->wptr = 0;
// Get/Set size of vector
#define bv_size(vec) \
    (vec->wptr)

// If declared as auto_vec_t there is no need to manually call on destroy
void _bv_destroy(bvec_t* vec_ref);
#define auto_bvec_t \
    bvec_t __attribute__((cleanup(_bv_destroy)))

#endif