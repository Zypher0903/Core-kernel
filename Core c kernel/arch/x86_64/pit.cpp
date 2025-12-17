#include <kernel/arch/x86_64/pit.h>
#include <kernel/arch/x86_64/io.h>

namespace Core {
namespace PIT {

static volatile uint64_t ticks = 0;

void initialize(uint32_t frequency) {
    uint32_t divisor = 1193182 / frequency;
    
    IO::outb(0x43, 0x36);
    IO::outb(0x40, divisor & 0xFF);
    IO::outb(0x40, (divisor >> 8) & 0xFF);
}

uint64_t get_ticks() {
    return ticks;
}

void sleep(uint32_t ms) {
    uint64_t end_tick = ticks + ms;
    while (ticks < end_tick) {
        __asm__ volatile("hlt");
    }
}

extern "C" void pit_tick() {
    ticks++;
}

}
}