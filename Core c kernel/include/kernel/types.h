#ifndef CORE_TYPES_H
#define CORE_TYPES_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef uint64_t size_t;
typedef int64_t ssize_t;
typedef uint64_t uintptr_t;
typedef int64_t intptr_t;

typedef uint8_t bool;
#define true 1
#define false 0

#define NULL ((void*)0)

#define PACKED __attribute__((packed))
#define ALIGNED(x) __attribute__((aligned(x)))

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define BIT(n) (1ULL << (n))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define offsetof(type, member) __builtin_offsetof(type, member)

typedef enum {
    E_OK = 0,
    E_NOMEM = -1,
    E_INVAL = -2,
    E_PERM = -3,
    E_NOENT = -4,
} error_t;

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

#define KERNEL_VIRTUAL_BASE 0xFFFFFFFF80000000ULL
#define KERNEL_HEAP_START   0xFFFFFFFF90000000ULL
#define KERNEL_HEAP_SIZE    (512ULL * 1024 * 1024)

#endif