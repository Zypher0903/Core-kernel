; Core Kernel Boot Code - Multiboot2

section .multiboot
align 8
multiboot_header_start:
    dd 0xe85250d6                ; Magic
    dd 0                         ; Architecture
    dd multiboot_header_end - multiboot_header_start
    dd -(0xe85250d6 + 0 + (multiboot_header_end - multiboot_header_start))

    ; Framebuffer tag
    align 8
    dw 5
    dw 0
    dd 20
    dd 1024
    dd 768
    dd 32

    ; End tag
    align 8
    dw 0
    dw 0
    dd 8
multiboot_header_end:

section .bss
align 16
stack_bottom:
    resb 65536
stack_top:

align 4096
global boot_pml4
boot_pml4:
    resb 4096

global boot_pdpt
boot_pdpt:
    resb 4096

global boot_pd
boot_pd:
    resb 4096

section .data
align 8
global boot_gdt64
boot_gdt64:
    dq 0x0000000000000000
    dq 0x00AF9A000000FFFF
    dq 0x00AF92000000FFFF
boot_gdt64_end:

boot_gdt64_ptr:
    dw boot_gdt64_end - boot_gdt64 - 1
    dq boot_gdt64

section .text.boot
bits 32

global _start
extern kernel_main
extern __init_array_start
extern __init_array_end

_start:
    cli
    mov edi, eax
    mov esi, ebx
    mov esp, stack_top

    call check_long_mode
    test eax, eax
    jz .no_long_mode

    call setup_page_tables

    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Load PML4
    mov eax, boot_pml4
    mov cr3, eax

    ; Enable long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    lgdt [boot_gdt64_ptr]
    jmp 0x08:long_mode_start

.no_long_mode:
    mov dword [0xb8000], 0x4f524f45
    hlt

check_long_mode:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    xor eax, ecx
    jz .no_cpuid

    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode

    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode

    mov eax, 1
    ret

.no_cpuid:
.no_long_mode:
    xor eax, eax
    ret

setup_page_tables:
    mov eax, boot_pdpt
    or eax, 0x3
    mov [boot_pml4], eax
    mov [boot_pml4 + 511 * 8], eax

    mov eax, boot_pd
    or eax, 0x3
    mov [boot_pdpt], eax
    mov [boot_pdpt + 510 * 8], eax

    mov ecx, 0
.map_pd:
    mov eax, 0x200000
    mul ecx
    or eax, 0x83
    mov [boot_pd + ecx * 8], eax
    inc ecx
    cmp ecx, 512
    jl .map_pd

    ret

bits 64
long_mode_start:
    mov ax, 0x10
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rax, stack_top
    add rax, 0xFFFFFFFF80000000
    mov rsp, rax

    mov rax, __init_array_start
    add rax, 0xFFFFFFFF80000000
    mov rbx, __init_array_end
    add rbx, 0xFFFFFFFF80000000
.call_constructors:
    cmp rax, rbx
    je .constructors_done
    push rax
    push rbx
    call [rax]
    pop rbx
    pop rax
    add rax, 8
    jmp .call_constructors

.constructors_done:
    mov rax, rsi
    add rax, 0xFFFFFFFF80000000
    mov rsi, rax
    
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang

global gdt_flush
gdt_flush:
    lgdt [rdi]
    ret

global tss_flush
tss_flush:
    mov ax, di
    ltr ax
    ret