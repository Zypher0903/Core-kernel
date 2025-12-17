#ifndef CORE_PMM_H
#define CORE_PMM_H

#include <kernel/types.h>
#include <kernel/sync/spinlock.h>

namespace Core {

class PMM {
public:
    static void initialize(uint64_t total_memory, uint64_t kernel_end);
    static uint64_t alloc_page();
    static uint64_t alloc_pages(size_t count);
    static void free_page(uint64_t addr);
    static void free_pages(uint64_t addr, size_t count);
    static uint64_t get_total_memory();
    static uint64_t get_used_memory();
    static uint64_t get_free_memory();
    static void mark_region_used(uint64_t start, uint64_t end);
    static void mark_region_free(uint64_t start, uint64_t end);
    
private:
    static constexpr size_t MAX_ORDER = 11;
    
    struct FreeBlock {
        FreeBlock* next;
        size_t order;
    };
    
    static FreeBlock* free_lists[MAX_ORDER];
    static uint64_t* bitmap;
    static size_t total_pages;
    static size_t free_pages;
    static Spinlock lock;
    
    static size_t get_order(size_t pages);
    static void split_block(uint64_t addr, size_t order);
    static uint64_t try_merge_buddy(uint64_t addr, size_t order);
    static uint64_t get_buddy(uint64_t addr, size_t order);
    static bool is_page_free(uint64_t page_num);
    static void set_page_used(uint64_t page_num);
    static void set_page_free(uint64_t page_num);
};

}

#endif