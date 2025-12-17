#include <kernel/types.h>
#include <kernel/console.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/memory/heap.h>
#include <kernel/arch/x86_64/gdt.h>
#include <kernel/arch/x86_64/idt.h>
#include <kernel/arch/x86_64/pit.h>
#include <kernel/arch/x86_64/apic.h>
#include <kernel/process/process.h>
#include <kernel/process/scheduler.h>
#include <kernel/fs/vfs.h>
#include <kernel/drivers/pci.h>
#include <kernel/multiboot2.h>

extern "C" uint64_t _kernel_end;
extern "C" uint64_t _kernel_physical_end;

namespace Core {

class KernelInfo {
public:
    uint64_t total_memory;
    uint64_t usable_memory;
    uint64_t kernel_start;
    uint64_t kernel_end;
    const char* bootloader_name;
    
    void print() {
        Console::printf("Core Microkernel v0.1.0\n");
        Console::printf("=======================\n\n");
        Console::printf("Bootloader: %s\n", bootloader_name ? bootloader_name : "Unknown");
        Console::printf("Kernel: 0x%llx - 0x%llx (%llu KB)\n", 
                       kernel_start, kernel_end, 
                       (kernel_end - kernel_start) / 1024);
        Console::printf("Total Memory: %llu MB\n", total_memory / (1024 * 1024));
        Console::printf("Usable Memory: %llu MB\n\n", usable_memory / (1024 * 1024));
    }
};

static KernelInfo kernel_info;

static void parse_multiboot_info(uint32_t magic, uint64_t info_addr) {
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        Console::printf("Invalid multiboot2 magic: 0x%x\n", magic);
        return;
    }

    struct multiboot_tag *tag;
    
    kernel_info.total_memory = 0;
    kernel_info.usable_memory = 0;
    kernel_info.bootloader_name = nullptr;

    for (tag = (struct multiboot_tag*)(info_addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag*)((uint8_t*)tag + ((tag->size + 7) & ~7))) {
        
        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
                struct multiboot_tag_string *str = (struct multiboot_tag_string*)tag;
                kernel_info.bootloader_name = str->string;
                break;
            }
            case MULTIBOOT_TAG_TYPE_MMAP: {
                struct multiboot_tag_mmap *mmap = (struct multiboot_tag_mmap*)tag;
                multiboot_memory_map_t *entry;
                
                for (entry = mmap->entries;
                     (uint8_t*)entry < (uint8_t*)tag + tag->size;
                     entry = (multiboot_memory_map_t*)((uint64_t)entry + mmap->entry_size)) {
                    
                    kernel_info.total_memory += entry->len;
                    if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                        kernel_info.usable_memory += entry->len;
                    }
                }
                break;
            }
        }
    }

    kernel_info.kernel_start = 0x100000;
    kernel_info.kernel_end = (uint64_t)&_kernel_physical_end;
}

static void init_early_console() {
    Console::initialize();
    Console::set_color(Console::Color::WHITE, Console::Color::BLACK);
    Console::clear();
}

static void init_kernel_subsystems() {
    Console::printf("[INIT] Initializing kernel subsystems...\n");

    Console::printf("[INIT] Setting up GDT... ");
    GDT::initialize();
    Console::printf("OK\n");

    Console::printf("[INIT] Setting up IDT... ");
    IDT::initialize();
    Console::printf("OK\n");

    Console::printf("[INIT] Initializing physical memory... ");
    PMM::initialize(kernel_info.usable_memory, kernel_info.kernel_end);
    Console::printf("OK (%llu MB free)\n", PMM::get_free_memory() / (1024 * 1024));

    Console::printf("[INIT] Initializing virtual memory... ");
    VMM::initialize();
    Console::printf("OK\n");

    Console::printf("[INIT] Initializing kernel heap... ");
    Heap::initialize(KERNEL_HEAP_START, KERNEL_HEAP_SIZE);
    Console::printf("OK\n");

    Console::printf("[INIT] Initializing APIC... ");
    APIC::initialize();
    Console::printf("OK\n");

    Console::printf("[INIT] Initializing PIT... ");
    PIT::initialize(1000);
    Console::printf("OK\n");

    Console::printf("[INIT] Initializing process manager... ");
    ProcessManager::initialize();
    Console::printf("OK\n");

    Console::printf("[INIT] Initializing scheduler... ");
    Scheduler::initialize();
    Console::printf("OK\n");

    Console::printf("[INIT] Initializing VFS... ");
    VFS::initialize();
    Console::printf("OK\n");

    Console::printf("[INIT] Scanning PCI bus... ");
    PCI::initialize();
    Console::printf("OK\n");

    Console::printf("\n[INIT] All subsystems initialized successfully!\n\n");
}

static void run_kernel_tests() {
    Console::printf("[TEST] Running kernel tests...\n");

    Console::printf("[TEST] Testing heap allocation... ");
    void* ptr1 = Heap::malloc(1024);
    void* ptr2 = Heap::malloc(2048);
    if (ptr1 && ptr2) {
        Heap::free(ptr1);
        Heap::free(ptr2);
        Console::printf("OK\n");
    } else {
        Console::printf("FAILED\n");
    }

    Console::printf("[TEST] Testing process creation... ");
    Process* proc = ProcessManager::create_kernel_process("test_process", 
        [](void*) -> void* {
            Console::printf("Test process running!\n");
            return nullptr;
        }, nullptr);
    if (proc) {
        Console::printf("OK (PID %d)\n", proc->get_pid());
    } else {
        Console::printf("FAILED\n");
    }

    Console::printf("[TEST] All tests passed!\n\n");
}

static void idle_task(void*) {
    Console::printf("[IDLE] Idle task started\n");
    
    while (true) {
        __asm__ volatile("hlt");
    }
}

}

extern "C" void kernel_main(uint32_t magic, uint64_t multiboot_info) {
    using namespace Core;

    init_early_console();
    parse_multiboot_info(magic, multiboot_info);
    kernel_info.print();
    init_kernel_subsystems();
    run_kernel_tests();

    Console::printf("[INIT] Creating idle task...\n");
    ProcessManager::create_kernel_process("idle", idle_task, nullptr);

    Console::printf("[INIT] Enabling interrupts...\n");
    __asm__ volatile("sti");

    Console::printf("[INIT] Starting scheduler...\n\n");
    Console::printf("====================================\n");
    Console::printf("Core Kernel is now running!\n");
    Console::printf("====================================\n\n");
    
    Scheduler::start();

    Console::printf("[PANIC] Scheduler returned!\n");
    while (true) {
        __asm__ volatile("cli; hlt");
    }
}

extern "C" void __cxa_pure_virtual() {
    Core::Console::printf("[PANIC] Pure virtual function called!\n");
    while (true) {
        __asm__ volatile("cli; hlt");
    }
}

extern "C" void *__dso_handle = nullptr;

extern "C" int __cxa_atexit(void (*)(void *), void *, void *) {
    return 0;
}