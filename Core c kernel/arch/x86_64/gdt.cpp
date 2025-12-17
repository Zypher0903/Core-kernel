#include <kernel/arch/x86_64/gdt.h>
#include <kernel/console.h>

namespace Core {
namespace GDT {

struct GDTEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} PACKED;

struct TSSEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
    uint32_t base_upper;
    uint32_t reserved;
} PACKED;

struct GDTPointer {
    uint16_t limit;
    uint64_t base;
} PACKED;

struct TSS {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} PACKED;

static GDTEntry gdt[7];
static TSSEntry* tss_entry;
static TSS tss;
static GDTPointer gdt_ptr;

extern "C" void gdt_flush(uint64_t);

void initialize() {
    gdt[0] = {0, 0, 0, 0, 0, 0};
    
    gdt[1].limit_low = 0xFFFF;
    gdt[1].base_low = 0;
    gdt[1].base_middle = 0;
    gdt[1].access = 0x9A;
    gdt[1].granularity = 0xAF;
    gdt[1].base_high = 0;
    
    gdt[2].limit_low = 0xFFFF;
    gdt[2].base_low = 0;
    gdt[2].base_middle = 0;
    gdt[2].access = 0x92;
    gdt[2].granularity = 0xCF;
    gdt[2].base_high = 0;
    
    gdt[3].limit_low = 0xFFFF;
    gdt[3].base_low = 0;
    gdt[3].base_middle = 0;
    gdt[3].access = 0xFA;
    gdt[3].granularity = 0xAF;
    gdt[3].base_high = 0;
    
    gdt[4].limit_low = 0xFFFF;
    gdt[4].base_low = 0;
    gdt[4].base_middle = 0;
    gdt[4].access = 0xF2;
    gdt[4].granularity = 0xCF;
    gdt[4].base_high = 0;
    
    tss_entry = (TSSEntry*)&gdt[5];
    uint64_t tss_base = (uint64_t)&tss;
    uint32_t tss_limit = sizeof(TSS) - 1;
    
    tss_entry->limit_low = tss_limit & 0xFFFF;
    tss_entry->base_low = tss_base & 0xFFFF;
    tss_entry->base_middle = (tss_base >> 16) & 0xFF;
    tss_entry->access = 0x89;
    tss_entry->granularity = 0x00;
    tss_entry->base_high = (tss_base >> 24) & 0xFF;
    tss_entry->base_upper = (tss_base >> 32) & 0xFFFFFFFF;
    tss_entry->reserved = 0;
    
    for (int i = 0; i < 7; i++) {
        tss.ist[i] = 0;
    }
    tss.iomap_base = sizeof(TSS);
    
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint64_t)&gdt;
    gdt_flush((uint64_t)&gdt_ptr);
}

void install_tss(uint64_t rsp0) {
    tss.rsp0 = rsp0;
}

}
}