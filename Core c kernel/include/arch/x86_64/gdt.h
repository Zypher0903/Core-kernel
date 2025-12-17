#ifndef CORE_GDT_H
#define CORE_GDT_H

#include <kernel/types.h>

namespace Core {
namespace GDT {

void initialize();
void install_tss(uint64_t rsp0);

}
}

#endif