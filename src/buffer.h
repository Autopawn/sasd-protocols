#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdint.h>

// Default size to allocate with buffer_create_default()
#define BUFFER_DEFAULT_SIZE 512

// Special flag for buffer_pop_string()
#define BUFFER_ALLOC_STRING -1

// Opaque data structure
struct _buffer;

// Convenient data type
typedef struct _buffer * buffer_t;

/*
 * Create new buffer with @size bytes
 */
buffer_t buffer_create(size_t size);

/*
 * Create new buffer with BUFFER_DEFAULT_SIZE
 * bytes
 */
buffer_t buffer_create_default();

/*
 * Create buffer from @data with @size bytes
 * This copies the data into the buffer
 */
buffer_t buffer_from_data(const void *data, size_t size);

/*
 * Returns read-only data within the buffer
 */
const char *buffer_data(const buffer_t);

/*
 * Returns number of written bytes in buffer
 */
size_t buffer_size(const buffer_t);

/*
 * Push char into the buffer
 *
 * Returns -1 on error (could not allocate)
 * Returns 0 if ok
 */
int buffer_push_char(buffer_t, char);

/*
 * Push 2-byte integer into the buffer
 *
 * Returns -1 on error (could not allocate)
 * Returns 0 if ok
 */
int buffer_push_int16(buffer_t, int16_t);

/*
 * Push 4-byte integer into the buffer
 *
 * Returns -1 on error (could not allocate)
 * Returns 0 if ok
 */
int buffer_push_int32(buffer_t, int32_t);

/*
 * Push 4-byte floating point into the buffer
 *
 * Returns -1 on error (could not allocate)
 * Returns 0 if ok
 */
int buffer_push_float32(buffer_t, float);

/*
 * Push 8-byte floating point into the buffer
 *
 * Returns -1 on error (could not allocate)
 * Returns 0 if ok
 */
int buffer_push_float64(buffer_t, double);

/*
 * Push c-string into the buffer; this is equivalent
 * to:
 *
 * buffer_push_int16(buffer, strlen(@string));
 * for (int i = 0; i < strlen(@string); i++) {
 *     buffer_push_char(@string[i]);
 * }
 *
 * Returns -1 on error (could not allocate)
 * Returns 0 if ok
 */
int buffer_push_string(buffer_t, const char *string);

/*
 * Pops char from buffer
 */
int buffer_pop_char(buffer_t, char *);

/*
 * Pops 2-byte integer from buffer
 *
 * Returns -1 on error (not enough data in buffer)
 * Returns 0 if could retrieve
 */
int buffer_pop_int16(buffer_t, int16_t *);

/*
 * Pops 4-byte integer from buffer
 *
 * Returns -1 on error (not enough data in buffer)
 * Returns 0 if could retrieve
 */
int buffer_pop_int32(buffer_t, int32_t *);

/*
 * Pops 4-byte floating point from buffer
 *
 * Returns -1 on error (not enough data in buffer)
 * Returns 0 if could retrieve
 */
int buffer_pop_float32(buffer_t, float *);

/*
 * Pops 8-byte floating point from buffer
 *
 * Returns -1 on error (not enough data in buffer)
 * Returns 0 if could retrieve
 */
int buffer_pop_float64(buffer_t, double *);

/*
 * Pops c-string from buffer. It may write to an
 * user-allocated space, or it can allocate enough
 * space to write the whole string.
 * Example:
 *
 * // User allocated
 * char string[256];
 * buffer_pop_string(buffer, string, 256);
 *
 * This is, the buffer and its size are passed as
 * parameters.
 *
 * // Auto allocate
 * char *string;
 * buffer_pop_string(buffer, &string, BUFFER_ALLOC_STRING);
 * ...
 * free(string);
 *
 * This is, a pointer to the buffer is passed as first
 * parameter, and BUFFER_ALLOC_STRING as special second parameter.
 * In this case, the user is responsible for freeing
 * the allocated string.
 *
 * Returns -1 on error (not enough data in buffer)
 * Returns >= 0 indicating length of the string
 */
int buffer_pop_string(buffer_t, void *, int);

/*
 * Destroy buffer
 */
void buffer_destroy(buffer_t);

/*
 * Definition of auto_buffer_t, this is useful if the user
 * wants the buffer to be destroyed after scope exit.
 * Example:
 *
 * {
 *     auto_buffer_t buffer = buffer_create_default();
 *     ...
 *     // there is no need to call buffer_destroy manually
 * }
 *
 * {
 *     buffer_t buffer = buffer_create_default();
 *     ...
 *     // Must call buffer_destroy
 *     buffer_destroy(buffer);
 * }
 */
void _destroy_buffer(buffer_t *);
#define auto_buffer_t\
    __attribute__((cleanup (_destroy_buffer))) buffer_t

#endif