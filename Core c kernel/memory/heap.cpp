#include <kernel/memory/heap.h>
#include <kernel/memory/vmm.h>
#include <kernel/memory/pmm.h>
#include <kernel/sync/spinlock.h>

namespace Core {

struct HeapBlock {
    size_t size;
    bool free;
    HeapBlock* next;
    HeapBlock* prev;
};

static uint64_t heap_start = 0;
static uint64_t heap_end = 0;
static HeapBlock* heap_head = nullptr;
static Spinlock heap_lock;
static size_t used_memory = 0;

void Heap::initialize(uint64_t start, uint64_t size) {
    heap_start = start;
    heap_end = start + size;
    
    for (uint64_t addr = heap_start; addr < heap_start + (16 * PAGE_SIZE); addr += PAGE_SIZE) {
        uint64_t phys = PMM::alloc_page();
        VMM::map_page(addr, phys, VMM::PRESENT | VMM::WRITABLE);
    }
    
    heap_head = (HeapBlock*)heap_start;
    heap_head->size = (16 * PAGE_SIZE) - sizeof(HeapBlock);
    heap_head->free = true;
    heap_head->next = nullptr;
    heap_head->prev = nullptr;
    
    used_memory = 0;
}

void* Heap::malloc(size_t size) {
    if (size == 0) return nullptr;
    
    ScopedLock guard(heap_lock);
    
    size = ALIGN_UP(size, 16);
    
    HeapBlock* block = heap_head;
    while (block) {
        if (block->free && block->size >= size) {
            if (block->size >= size + sizeof(HeapBlock) + 64) {
                HeapBlock* new_block = (HeapBlock*)((uint64_t)block + sizeof(HeapBlock) + size);
                new_block->size = block->size - size - sizeof(HeapBlock);
                new_block->free = true;
                new_block->next = block->next;
                new_block->prev = block;
                
                if (block->next) {
                    block->next->prev = new_block;
                }
                
                block->size = size;
                block->next = new_block;
            }
            
            block->free = false;
            used_memory += block->size;
            
            return (void*)((uint64_t)block + sizeof(HeapBlock));
        }
        block = block->next;
    }
    
    return nullptr;
}

void* Heap::calloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = malloc(total);
    
    if (ptr) {
        uint8_t* bytes = (uint8_t*)ptr;
        for (size_t i = 0; i < total; i++) {
            bytes[i] = 0;
        }
    }
    
    return ptr;
}

void* Heap::realloc(void* ptr, size_t new_size) {
    if (!ptr) return malloc(new_size);
    if (new_size == 0) {
        free(ptr);
        return nullptr;
    }
    
    HeapBlock* block = (HeapBlock*)((uint64_t)ptr - sizeof(HeapBlock));
    
    if (block->size >= new_size) {
        return ptr;
    }
    
    void* new_ptr = malloc(new_size);
    if (!new_ptr) return nullptr;
    
    uint8_t* src = (uint8_t*)ptr;
    uint8_t* dst = (uint8_t*)new_ptr;
    for (size_t i = 0; i < block->size; i++) {
        dst[i] = src[i];
    }
    
    free(ptr);
    return new_ptr;
}

void Heap::free(void* ptr) {
    if (!ptr) return;
    
    ScopedLock guard(heap_lock);
    
    HeapBlock* block = (HeapBlock*)((uint64_t)ptr - sizeof(HeapBlock));
    block->free = true;
    used_memory -= block->size;
    
    if (block->next && block->next->free) {
        block->size += sizeof(HeapBlock) + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
    
    if (block->prev && block->prev->free) {
        block->prev->size += sizeof(HeapBlock) + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}

size_t Heap::get_used() {
    return used_memory;
}

size_t Heap::get_free() {
    ScopedLock guard(heap_lock);
    
    size_t free_memory = 0;
    HeapBlock* block = heap_head;
    while (block) {
        if (block->free) {
            free_memory += block->size;
        }
        block = block->next;
    }
    
    return free_memory;
}

}