#ifndef CORE_SCHEDULER_H
#define CORE_SCHEDULER_H

namespace Core {

class Scheduler {
public:
    static void initialize();
    static void start();
    static void yield();
};

}

#endif