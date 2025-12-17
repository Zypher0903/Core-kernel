#include <kernel/arch/x86_64/idt.h>
#include <kernel/console.h>

namespace Core {
namespace IDT {

struct IDTEntry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} PACKED;

struct IDTPointer {
    uint16_t limit;
    uint64_t base;
} PACKED;

static IDTEntry idt[256];
static IDTPointer idt_ptr;

void set_gate(uint8_t num, uint64_t handler, uint8_t ist) {
    idt[num].offset_low = handler & 0xFFFF;
    idt[num].selector = 0x08;
    idt[num].ist = ist;
    idt[num].type_attr = 0x8E;
    idt[num].offset_mid = (handler >> 16) & 0xFFFF;
    idt[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[num].zero = 0;
}

void initialize() {
    for (int i = 0; i < 256; i++) {
        idt[i] = {0, 0, 0, 0, 0, 0, 0};
    }
    
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint64_t)&idt;
    
    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}

}
}