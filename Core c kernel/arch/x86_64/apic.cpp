#include <kernel/arch/x86_64/apic.h>

namespace Core {
namespace APIC {

#define IA32_APIC_BASE_MSR 0x1B

static uint64_t lapic_base = 0;

static uint32_t read_lapic(uint32_t reg) {
    return *((volatile uint32_t*)(lapic_base + reg));
}

static void write_lapic(uint32_t reg, uint32_t value) {
    *((volatile uint32_t*)(lapic_base + reg)) = value;
}

void initialize() {
    uint32_t eax, edx;
    __asm__ volatile("rdmsr" : "=a"(eax), "=d"(edx) : "c"(IA32_APIC_BASE_MSR));
    lapic_base = ((uint64_t)edx << 32) | (eax & 0xFFFFF000);
    lapic_base += 0xFFFFFFFF80000000;
    
    write_lapic(0xF0, read_lapic(0xF0) | 0x1FF);
}

void send_eoi() {
    write_lapic(0xB0, 0);
}

void send_ipi(uint32_t cpu, uint8_t vector) {
    write_lapic(0x310, (cpu << 24));
    write_lapic(0x300, vector);
}

}
}