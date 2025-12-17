#include <kernel/types.h>
#include <kernel/console.h>
#include <kernel/arch/x86_64/apic.h>

namespace Core {

struct InterruptFrame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_num, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

static const char* exception_messages[] = {
    "Division By Zero", "Debug", "NMI", "Breakpoint",
    "Overflow", "Bound Range", "Invalid Opcode", "Device Not Available",
    "Double Fault", "Coprocessor", "Invalid TSS", "Segment Not Present",
    "Stack Fault", "General Protection", "Page Fault", "Reserved",
    "FPU Error", "Alignment Check", "Machine Check", "SIMD Exception"
};

extern "C" void interrupt_handler(InterruptFrame* frame) {
    if (frame->int_num < 32) {
        Console::printf("\n[EXCEPTION] %s (#%llu)\n", 
                       exception_messages[frame->int_num], frame->int_num);
        Console::printf("Error Code: 0x%llx\n", frame->error_code);
        Console::printf("RIP: 0x%llx\n", frame->rip);
        
        if (frame->int_num == 14) {
            uint64_t cr2;
            __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
            Console::printf("Page Fault Address: 0x%llx\n", cr2);
        }
        
        while (true) {
            __asm__ volatile("cli; hlt");
        }
    } else if (frame->int_num >= 32 && frame->int_num < 48) {
        if (frame->int_num == 32) {
            extern void pit_tick();
            pit_tick();
        }
        
        APIC::send_eoi();
    }
}

}