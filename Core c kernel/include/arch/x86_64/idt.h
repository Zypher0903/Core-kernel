#ifndef CORE_IDT_H
#define CORE_IDT_H

#include <kernel/types.h>

namespace Core {
namespace IDT {

void initialize();
void set_gate(uint8_t num, uint64_t handler, uint8_t ist);

}
}

#endif