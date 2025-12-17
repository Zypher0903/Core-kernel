#include <kernel/process/process.h>
#include <kernel/memory/heap.h>
#include <kernel/console.h>

namespace Core {

static int next_pid = 1;

struct ProcessData {
    int pid;
    const char* name;
    thread_func_t func;
    void* arg;
    uint64_t stack;
    bool running;
};

#define MAX_PROCESSES 256
static ProcessData processes[MAX_PROCESSES];
static int process_count = 0;
static ProcessData* current_process = nullptr;

void ProcessManager::initialize() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].pid = 0;
        processes[i].running = false;
    }
    process_count = 0;
}

Process* ProcessManager::create_kernel_process(const char* name, thread_func_t func, void* arg) {
    if (process_count >= MAX_PROCESSES) {
        return nullptr;
    }
    
    ProcessData* proc = &processes[process_count++];
    proc->pid = next_pid++;
    proc->name = name;
    proc->func = func;
    proc->arg = arg;
    proc->stack = (uint64_t)Heap::malloc(64 * 1024);
    proc->running = true;
    
    return (Process*)proc;
}

Process* ProcessManager::get_current() {
    return (Process*)current_process;
}

void ProcessManager::exit(int code) {
    if (current_process) {
        Console::printf("[PROC] Process %d exited with code %d\n", 
                       current_process->pid, code);
        current_process->running = false;
    }
}

}