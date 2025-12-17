#ifndef CORE_VMM_H
#define CORE_VMM_H

#include <kernel/types.h>

namespace Core {

class VMM {
public:
    static void initialize();
    static void* map_page(uint64_t virt, uint64_t phys, uint32_t flags);
    static void unmap_page(uint64_t virt);
    static uint64_t virt_to_phys(uint64_t virt);
    
    enum Flags {
        PRESENT = 1 << 0,
        WRITABLE = 1 << 1,
        USER = 1 << 2,
        NO_EXECUTE = 1ULL << 63
    };
};

}

#endif