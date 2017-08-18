#include <stdio.h>
#include <stdint.h>

struct object_state {
    int32_t x, y, vx, vy;
    char type;
};

/*
 * Types
 * - o => object_state
 */
size_t calc_stream_size(const char *type)
{
    size_t size = 0;
    for (; *type != 0; type++) {
        switch (*type) {
            case 'i':
                size += 4;
                break;
            case 'c':
                size += 1;
                break;
            case 'o':
                // int32_t[4] + char
                size += calc_stream_size("iiiic");
                break;
            default:
                size += 0;
        }
    }
    return size;
}

void push_data(char *stream, void *data)
{

}

void push_i32(char *stream, int32_t i32)
{

}

int main(int argc, char *argv[])
{
    printf("Hello world!\n");
}