/* Core Kernel Linker Script */

ENTRY(_start)

SECTIONS
{
    . = 0xFFFFFFFF80000000 + 0x100000;

    .text ALIGN(4K) : AT(ADDR(.text) - 0xFFFFFFFF80000000)
    {
        KEEP(*(.multiboot))
        *(.text.boot)
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xFFFFFFFF80000000)
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - 0xFFFFFFFF80000000)
    {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - 0xFFFFFFFF80000000)
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    .init_array ALIGN(4K) : AT(ADDR(.init_array) - 0xFFFFFFFF80000000)
    {
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
    }

    . = ALIGN(4K);
    _kernel_end = .;
    _kernel_physical_end = . - 0xFFFFFFFF80000000;

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
    }
}
```

### `.gitignore`
```
build/
*.o
*.elf
*.iso
*.bin
*.swp
*~
.vscode/
.DS_Store