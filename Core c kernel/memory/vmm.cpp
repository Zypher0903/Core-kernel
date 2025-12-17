#include <kernel/memory/vmm.h>
#include <kernel/memory/pmm.h>

namespace Core {

static uint64_t* kernel_pml4 = nullptr;

void VMM::initialize() {
    __asm__ volatile("mov %%cr3, %0" : "=r"(kernel_pml4));
    kernel_pml4 = (uint64_t*)((uint64_t)kernel_pml4 + KERNEL_VIRTUAL_BASE);
}

void* VMM::map_page(uint64_t virt, uint64_t phys, uint32_t flags) {
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx = (virt >> 21) & 0x1FF;
    uint64_t pt_idx = (virt >> 12) & 0x1FF;
    
    uint64_t* pdpt;
    if (!(kernel_pml4[pml4_idx] & PRESENT)) {
        pdpt = (uint64_t*)(PMM::alloc_page() + KERNEL_VIRTUAL_BASE);
        for (int i = 0; i < 512; i++) pdpt[i] = 0;
        kernel_pml4[pml4_idx] = ((uint64_t)pdpt - KERNEL_VIRTUAL_BASE) | PRESENT | WRITABLE;
    } else {
        pdpt = (uint64_t*)((kernel_pml4[pml4_idx] & ~0xFFF) + KERNEL_VIRTUAL_BASE);
    }
    
    uint64_t* pd;
    if (!(pdpt[pdpt_idx] & PRESENT)) {
        pd = (uint64_t*)(PMM::alloc_page() + KERNEL_VIRTUAL_BASE);
        for (int i = 0; i < 512; i++) pd[i] = 0;
        pdpt[pdpt_idx] = ((uint64_t)pd - KERNEL_VIRTUAL_BASE) | PRESENT | WRITABLE;
    } else {
        pd = (uint64_t*)((pdpt[pdpt_idx] & ~0xFFF) + KERNEL_VIRTUAL_BASE);
    }
    
    uint64_t* pt;
    if (!(pd[pd_idx] & PRESENT)) {
        pt = (uint64_t*)(PMM::alloc_page() + KERNEL_VIRTUAL_BASE);
        for (int i = 0; i < 512; i++) pt[i] = 0;
        pd[pd_idx] = ((uint64_t)pt - KERNEL_VIRTUAL_BASE) | PRESENT | WRITABLE;
    } else {
        pt = (uint64_t*)((pd[pd_idx] & ~0xFFF) + KERNEL_VIRTUAL_BASE);
    }
    
    pt[pt_idx] = (phys & ~0xFFF) | flags;
    
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
    
    return (void*)virt;
}

void VMM::unmap_page(uint64_t virt) {
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx = (virt >> 21) & 0x1FF;
    uint64_t pt_idx = (virt >> 12) & 0x1FF;
    
    if (!(kernel_pml4[pml4_idx] & PRESENT)) return;
    uint64_t* pdpt = (uint64_t*)((kernel_pml4[pml4_idx] & ~0xFFF) + KERNEL_VIRTUAL_BASE);
    
    if (!(pdpt[pdpt_idx] & PRESENT)) return;
    uint64_t* pd = (uint64_t*)((pdpt[pdpt_idx] & ~0xFFF) + KERNEL_VIRTUAL_BASE);
    
    if (!(pd[pd_idx] & PRESENT)) return;
    uint64_t* pt = (uint64_t*)((pd[pd_idx] & ~0xFFF) + KERNEL_VIRTUAL_BASE);
    
    pt[pt_idx] = 0;
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

uint64_t VMM::virt_to_phys(uint64_t virt) {
    uint64_t pml4_idx = (virt >> 39) & 0x1FF;
    uint64_t pdpt_idx = (virt >> 30) & 0x1FF;
    uint64_t pd_idx = (virt >> 21) & 0x1FF;
    uint64_t pt_idx = (virt >> 12) & 0x1FF;
    uint64_t offset = virt & 0xFFF;
    
    if (!(kernel_pml4[pml4_idx] & PRESENT)) return 0;
    uint64_t* pdpt = (uint64_t*)((kernel_pml4[pml4_idx] & ~0xFFF) + KERNEL_VIRTUAL_BASE);
    
    if (!(pdpt[pdpt_idx] & PRESENT)) return 0;
    uint64_t* pd = (uint64_t*)((pdpt[pdpt_idx] & ~0xFFF) + KERNEL_VIRTUAL_BASE);
    
    if (!(pd[pd_idx] & PRESENT)) return 0;
    uint64_t* pt = (uint64_t*)((pd[pd_idx] & ~0xFFF) + KERNEL_VIRTUAL_BASE);
    
    if (!(pt[pt_idx] & PRESENT)) return 0;
    
    return (pt[pt_idx] & ~0xFFF) | offset;
}

}