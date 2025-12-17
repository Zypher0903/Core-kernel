#ifndef CORE_SPINLOCK_H
#define CORE_SPINLOCK_H

#include <kernel/types.h>

namespace Core {

class Spinlock {
public:
    Spinlock() : locked(0) {}
    
    void lock() {
        while (__atomic_test_and_set(&locked, __ATOMIC_ACQUIRE)) {
            while (__atomic_load_n(&locked, __ATOMIC_RELAXED)) {
                __asm__ volatile("pause");
            }
        }
    }
    
    void unlock() {
        __atomic_clear(&locked, __ATOMIC_RELEASE);
    }
    
    bool try_lock() {
        return !__atomic_test_and_set(&locked, __ATOMIC_ACQUIRE);
    }
    
private:
    volatile int locked;
};

class ScopedLock {
public:
    explicit ScopedLock(Spinlock& lock) : lock(lock) {
        lock.lock();
    }
    
    ~ScopedLock() {
        lock.unlock();
    }
    
private:
    Spinlock& lock;
};

}

#endif