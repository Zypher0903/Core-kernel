#include <kernel/memory/pmm.h>
#include <kernel/console.h>

namespace Core {

PMM::FreeBlock* PMM::free_lists[MAX_ORDER];
uint64_t* PMM::bitmap = nullptr;
size_t PMM::total_pages = 0;
size_t PMM::free_pages = 0;
Spinlock PMM::lock;

void PMM::initialize(uint64_t total_memory, uint64_t kernel_end) {
    total_pages = total_memory / PAGE_SIZE;
    
    uint64_t bitmap_size = (total_pages + 63) / 64 * sizeof(uint64_t);
    bitmap = (uint64_t*)(kernel_end + KERNEL_VIRTUAL_BASE);
    
    for (size_t i = 0; i < (total_pages + 63) / 64; i++) {
        bitmap[i] = 0xFFFFFFFFFFFFFFFF;
    }
    
    for (size_t i = 0; i < MAX_ORDER; i++) {
        free_lists[i] = nullptr;
    }
    
    free_pages = 0;
    
    uint64_t available_start = ALIGN_UP(kernel_end + bitmap_size, PAGE_SIZE);
    uint64_t available_end = total_memory;
    
    mark_region_free(available_start, available_end);
}

uint64_t PMM::alloc_page() {
    return alloc_pages(1);
}

uint64_t PMM::alloc_pages(size_t count) {
    ScopedLock guard(lock);
    
    if (count == 0) return 0;
    
    size_t order = get_order(count);
    if (order >= MAX_ORDER) return 0;
    
    size_t current_order = order;
    while (current_order < MAX_ORDER && !free_lists[current_order]) {
        current_order++;
    }
    
    if (current_order >= MAX_ORDER) {
        return 0;
    }
    
    FreeBlock* block = free_lists[current_order];
    free_lists[current_order] = block->next;
    uint64_t addr = (uint64_t)block;
    
    while (current_order > order) {
        split_block(addr, current_order);
        current_order--;
    }
    
    size_t pages = 1 << order;
    for (size_t i = 0; i < pages; i++) {
        set_page_used((addr / PAGE_SIZE) + i);
    }
    
    free_pages -= pages;
    
    return addr;
}

void PMM::free_page(uint64_t addr) {
    free_pages(addr, 1);
}

void PMM::free_pages(uint64_t addr, size_t count) {
    ScopedLock guard(lock);
    
    if (!addr || count == 0) return;
    
    size_t order = get_order(count);
    if (order >= MAX_ORDER) return;
    
    size_t pages = 1 << order;
    for (size_t i = 0; i < pages; i++) {
        set_page_free((addr / PAGE_SIZE) + i);
    }
    
    free_pages += pages;
    
    addr = try_merge_buddy(addr, order);
    
    FreeBlock* block = (FreeBlock*)addr;
    block->order = order;
    block->next = free_lists[order];
    free_lists[order] = block;
}

void PMM::mark_region_used(uint64_t start, uint64_t end) {
    start = ALIGN_DOWN(start, PAGE_SIZE);
    end = ALIGN_UP(end, PAGE_SIZE);
    
    for (uint64_t addr = start; addr < end; addr += PAGE_SIZE) {
        set_page_used(addr / PAGE_SIZE);
    }
}

void PMM::mark_region_free(uint64_t start, uint64_t end) {
    start = ALIGN_DOWN(start, PAGE_SIZE);
    end = ALIGN_UP(end, PAGE_SIZE);
    
    for (uint64_t addr = start; addr < end; ) {
        size_t order = MAX_ORDER - 1;
        while (order > 0) {
            size_t block_pages = 1 << order;
            if (addr + (block_pages * PAGE_SIZE) <= end &&
                (addr % (block_pages * PAGE_SIZE)) == 0) {
                break;
            }
            order--;
        }
        
        size_t block_pages = 1 << order;
        
        for (size_t i = 0; i < block_pages; i++) {
            set_page_free((addr / PAGE_SIZE) + i);
        }
        
        FreeBlock* block = (FreeBlock*)addr;
        block->order = order;
        block->next = free_lists[order];
        free_lists[order] = block;
        
        free_pages += block_pages;
        addr += block_pages * PAGE_SIZE;
    }
}

uint64_t PMM::get_total_memory() {
    return total_pages * PAGE_SIZE;
}

uint64_t PMM::get_used_memory() {
    return (total_pages - free_pages) * PAGE_SIZE;
}

uint64_t PMM::get_free_memory() {
    return free_pages * PAGE_SIZE;
}

size_t PMM::get_order(size_t pages) {
    size_t order = 0;
    size_t size = 1;
    while (size < pages && order < MAX_ORDER - 1) {
        size <<= 1;
        order++;
    }
    return order;
}

void PMM::split_block(uint64_t addr, size_t order) {
    if (order == 0) return;
    
    size_t new_order = order - 1;
    size_t block_size = (1 << new_order) * PAGE_SIZE;
    
    uint64_t buddy_addr = addr + block_size;
    FreeBlock* buddy = (FreeBlock*)buddy_addr;
    buddy->order = new_order;
    buddy->next = free_lists[new_order];
    free_lists[new_order] = buddy;
}

uint64_t PMM::try_merge_buddy(uint64_t addr, size_t order) {
    if (order >= MAX_ORDER - 1) return addr;
    
    uint64_t buddy_addr = get_buddy(addr, order);
    
    FreeBlock** prev = &free_lists[order];
    FreeBlock* current = free_lists[order];
    
    while (current) {
        if ((uint64_t)current == buddy_addr) {
            *prev = current->next;
            uint64_t merged_addr = MIN(addr, buddy_addr);
            return try_merge_buddy(merged_addr, order + 1);
        }
        prev = &current->next;
        current = current->next;
    }
    
    return addr;
}

uint64_t PMM::get_buddy(uint64_t addr, size_t order) {
    size_t block_size = (1 << order) * PAGE_SIZE;
    return addr ^ block_size;
}

bool PMM::is_page_free(uint64_t page_num) {
    if (page_num >= total_pages) return false;
    return !(bitmap[page_num / 64] & (1ULL << (page_num % 64)));
}

void PMM::set_page_used(uint64_t page_num) {
    if (page_num >= total_pages) return;
    bitmap[page_num / 64] |= (1ULL << (page_num % 64));
}

void PMM::set_page_free(uint64_t page_num) {
    if (page_num >= total_pages) return;
    bitmap[page_num / 64] &= ~(1ULL << (page_num % 64));
}

}