#define _LINUX
#include "memory.h"
#include <stdlib.h>
#include <exception>
#include <new>
#include <algorithm>

#define PATH_TO_IDENTIFIER ""

static char next = 1;

memory::memory(size_t size) : size(size)
{
    if (size <= 128) // you're retarded for asking for that little memory
        throw std::bad_alloc(); // throw something else?

    key = ftok(PATH_TO_IDENTIFIER, next);
    shmid = shmget(key, this->size, 0644 | IPC_CREAT);
    ++next %= 1 << (sizeof(next) << 3); // woah

    start = (char *) shmat(shmid, (void *) 0, 0);
    if (start == (void *) (-1))
        throw std::bad_alloc(); // same?

    end_ptr = (char **) start;

    // Here comes the rape
    map = (char **) ++end_ptr;
    map_size = (size >> 3); // mapping bytes
    *end_ptr += map_size >> sizeof(*end_ptr); // in bits

    for (char *itr = *map; itr < *end_ptr; itr++)
        *itr = 0;

    allocated = 0;
}

memory::~memory(void)
{
    shmctl(shmid, IPC_RMID, NULL);
}

#define IS_NULL(i, j) !(((size_t)(*map + ((i + j) & ~7)) >> ((i + j) % 8)) % 2)

size_t memory::next_free(size_t sz)
{
    size_t off = (size_t) ((*map + map_size) - start), j, i;
    for (i = 0; i < size; i++) {
        j = 0;
        while (!IS_NULL((size_t) off + i, 0) && i < size)
            i += 7;
        ++i;
        while (IS_NULL(off + i, j) && j < sz &&
                j < ((map_size << sizeof(**map)) - i))
            ++j;
        if (j == sz)
            return i;
    }
    return 0;
}

void *memory::alloc(size_t sz)
{
    if (sz >= size || allocated >= size)
        return NULL;

    size_t result = next_free(sz);
    if (!result)
        return (void *) result;

    char *buf;
    size_t b, r = result;
    while (sz) {
        b = 0;
        buf = *map + (sz + r);
        while (b < 8 && sz--)
            *buf = (1 << b++);
    }
    ++allocated;
    *end_ptr = std::max(start + result + sz, *end_ptr);

    return (void *) ((size_t) start + result);
}

void memory::free(void *ptr)
{
    if (!ptr || ptr < start ||
            ptr > (void *)((size_t) start + (size_t) *end_ptr))
        return;

    size_t off = (size_t) ptr - (size_t) start;
    char *j;
    while ((j = (char *)((size_t)(*map + (off & ~7)))) &&
            (*j >> (off % 8) % 2))
        *j &= ~(1 << (off++ % 8));

    --allocated;
}

