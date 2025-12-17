#ifndef CORE_APIC_H
#define CORE_APIC_H

#include <kernel/types.h>

namespace Core {
namespace APIC {

void initialize();
void send_eoi();
void send_ipi(uint32_t cpu, uint8_t vector);

}
}

#endif