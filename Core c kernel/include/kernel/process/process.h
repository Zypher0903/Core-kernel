#ifndef CORE_PROCESS_H
#define CORE_PROCESS_H

#include <kernel/types.h>

namespace Core {

typedef void* (*thread_func_t)(void*);

class Process {
public:
    int get_pid() const { return pid; }
    
private:
    int pid;
    friend class ProcessManager;
};

class ProcessManager {
public:
    static void initialize();
    static Process* create_kernel_process(const char* name, thread_func_t func, void* arg);
    static Process* get_current();
    static void exit(int code);
};

}

#endif