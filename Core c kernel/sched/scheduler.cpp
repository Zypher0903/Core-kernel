#include <kernel/process/scheduler.h>
#include <kernel/console.h>

namespace Core {

static bool scheduler_running = false;

void Scheduler::initialize() {
    scheduler_running = false;
}

void Scheduler::start() {
    scheduler_running = true;
    
    while (true) {
        __asm__ volatile("hlt");
    }
}

void Scheduler::yield() {
}

}