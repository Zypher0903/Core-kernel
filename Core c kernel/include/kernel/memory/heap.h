#ifndef CORE_HEAP_H
#define CORE_HEAP_H

#include <kernel/types.h>

namespace Core {

class Heap {
public:
    static void initialize(uint64_t start, uint64_t size);
    static void* malloc(size_t size);
    static void* calloc(size_t num, size_t size);
    static void* realloc(void* ptr, size_t new_size);
    static void free(void* ptr);
    static size_t get_used();
    static size_t get_free();
};

}

#endif