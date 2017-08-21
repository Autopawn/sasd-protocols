#include "buffer.h"

#include <stdlib.h> // malloc, free
#include <string.h> // memcpy, strlen

// If x == y then quit function returning v
#define VERIFY(x, y, v) if (x == y) return v;

#define MIN(x, y) ((x < y) ? x : y)

struct _buffer
{
    size_t rptr;
    size_t wptr;
    size_t size;
    char data[];
};

/*
 * true (1) if remaining capacity of buffer is enough
 * to write s more bytes, false (0) otherwise
 */
int __buffer_capacity_chkw(buffer_t buffer, size_t size) {
    return (size <= (buffer->size - buffer->wptr)) ? 1 : 0;
}

/*
 * true (1) if remaining capacity of buffer is enough
 * to read s more bytes, false (0) otherwise
 */
int __buffer_capacity_chkr(buffer_t buffer, size_t size) {
    return (size <= (buffer->size - buffer->rptr)) ? 1 : 0;
}

/*
 * Reallocs the buffer if current capacity < size
 */
buffer_t __buffer_realloc_if(buffer_t *buffer, size_t size)
{
    // If ok, there is no need to reallocate
    VERIFY(__buffer_capacity_chkw(*buffer, size), 1, *buffer);

    (*buffer)->size += (*buffer)->size / 2;
    *buffer = realloc(*buffer,
        sizeof(struct _buffer) + (*buffer)->size);

    return *buffer;
}

/*
 * Push @size bytes from @any into @buffer
 */
int __buffer_push_any(buffer_t buffer, const void *any, size_t size)
{
    VERIFY(__buffer_realloc_if(&buffer, size), NULL, -1);
    memcpy(buffer->data + buffer->wptr, any, size);
    buffer->wptr += size;
    return 0;
}

/*
 * Pop @size bytes from @buffer into @any
 */
int __buffer_pop_any(buffer_t buffer, void *any, size_t size)
{
    VERIFY(__buffer_capacity_chkr(buffer, size), 0, -1);
    memcpy(any, buffer->data + buffer->rptr, size);
    buffer->rptr += size;
    return 0;
}

buffer_t buffer_create(size_t size)
{
    buffer_t buffer = malloc(sizeof(struct _buffer) + size);
    buffer->rptr = 0;
    buffer->wptr = 0;
    buffer->size = size;
    return buffer;
}

buffer_t buffer_create_default()
{
    return buffer_create(BUFFER_DEFAULT_SIZE);
}

buffer_t buffer_from_data(const void *data, size_t size)
{
    buffer_t buffer = buffer_create(size);
    memcpy(buffer->data, data, size);
    buffer->wptr = size;
    return buffer;
}

const char *buffer_data(const buffer_t buffer)
{
    return buffer->data;
}

/*
 * Internally, the name can be a little misleading, but
 * what the user really matters is the size of the pushed data
 */
size_t buffer_size(const buffer_t buffer)
{
    return buffer->wptr;
}

int buffer_push_char(buffer_t buffer, char c)
{
    VERIFY(__buffer_realloc_if(&buffer, 1), NULL, -1);
    buffer->data[buffer->wptr++] = c;
    return 0;
}

int buffer_push_int16(buffer_t buffer, int16_t i16)
{
    int16_t n_i16 = htons(i16);
    return __buffer_push_any(buffer, &n_i16, sizeof(int16_t));
}

int buffer_push_int32(buffer_t buffer, int32_t i32)
{
    int32_t n_i32 = htonl(i32);
    return __buffer_push_any(buffer, &n_i32, sizeof(int32_t));
}

int buffer_push_float32(buffer_t buffer, float f32)
{
    return __buffer_push_any(buffer, &f32, sizeof(float));
}

int buffer_push_float64(buffer_t buffer, double f64)
{
    return __buffer_push_any(buffer, &f64, sizeof(double));
}

int buffer_push_string(buffer_t buffer, const char *string)
{
    int16_t length = strlen(string);
    VERIFY(buffer_push_int16(buffer, length), -1, -1);
    return __buffer_push_any(buffer, string, strlen(string));
}

int buffer_pop_char(buffer_t buffer, char *c)
{
    VERIFY(__buffer_capacity_chkr(buffer, 1), 0, -1);
    *c = buffer->data[buffer->rptr++];
    return 0;
}

int buffer_pop_int16(buffer_t buffer, int16_t *i16)
{
    VERIFY(__buffer_pop_any(buffer, i16, sizeof(int16_t)), -1, -1)
    *i16 = ntohs(*i16);
    return 0;
}

int buffer_pop_int32(buffer_t buffer, int32_t *i32)
{
    VERIFY(__buffer_pop_any(buffer, i32, sizeof(int32_t)), -1, -1)
    *i32 = ntohl(*i32);
    return 0;
}

int buffer_pop_float32(buffer_t buffer, float *f32)
{
    return __buffer_pop_any(buffer, f32, 4);
}

int buffer_pop_float64(buffer_t buffer, double *f64)
{
    return __buffer_pop_any(buffer, f64, 8);
}

int buffer_pop_string(buffer_t buffer, void *str, int size)
{
    int16_t length;
    int allocd = 0;
    VERIFY(buffer_pop_int16(buffer, &length), -1, -1);

    // dst string
    char *string;

    // Allocate (BUFFER_ALLOC_STRING)
    if (size < 0) {
        // Inferred type char**
        size = length;
        string = *((char**)str) = malloc(size + 1);
        allocd = 1;
    
    } else {
        // Inferred type char*
        size = MIN(size - 1, length);
        string = str;
    }

    if (__buffer_pop_any(buffer, string, size) == -1) {
        if (allocd) {
            free(string);
        }
        return -1;
    }
    string[size] = 0;

    return size;
}

void buffer_destroy(buffer_t buffer)
{
    free(buffer);
}

/*
 * Function to be called with RAII method (auto_buffer_t)
 */
void _destroy_buffer(buffer_t *buffref)
{
    if (buffref != NULL) {
        buffer_destroy(*buffref);
    }
}