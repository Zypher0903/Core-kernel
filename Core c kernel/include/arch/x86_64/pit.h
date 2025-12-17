#ifndef CORE_PIT_H
#define CORE_PIT_H

#include <kernel/types.h>

namespace Core {
namespace PIT {

void initialize(uint32_t frequency);
uint64_t get_ticks();
void sleep(uint32_t ms);

}
}

#endif