#ifndef CORE_MULTIBOOT2_H
#define CORE_MULTIBOOT2_H

#include <kernel/types.h>

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289

#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE           1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT_TAG_TYPE_MMAP              6

#define MULTIBOOT_MEMORY_AVAILABLE          1
#define MULTIBOOT_MEMORY_RESERVED           2

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
} PACKED;

struct multiboot_tag_string {
    uint32_t type;
    uint32_t size;
    char string[0];
} PACKED;

typedef struct multiboot_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} PACKED multiboot_memory_map_t;

struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    multiboot_memory_map_t entries[0];
} PACKED;

#endif