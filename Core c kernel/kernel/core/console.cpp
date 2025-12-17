#include <kernel/console.h>
#include <kernel/arch/x86_64/io.h>
#include <stdarg.h>

namespace Core {

size_t Console::row = 0;
size_t Console::column = 0;
uint8_t Console::color = 0x07;

void Console::initialize() {
    row = 0;
    column = 0;
    color = make_color(Color::WHITE, Color::BLACK);
}

void Console::clear() {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = make_vga_entry(' ', color);
        }
    }
    row = 0;
    column = 0;
    update_cursor();
}

uint8_t Console::make_color(Color fg, Color bg) {
    return (uint8_t)fg | ((uint8_t)bg << 4);
}

uint16_t Console::make_vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

void Console::set_color(Color fg, Color bg) {
    color = make_color(fg, bg);
}

void Console::scroll() {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[y * VGA_WIDTH + x] = VGA_MEMORY[(y + 1) * VGA_WIDTH + x];
        }
    }
    
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = make_vga_entry(' ', color);
    }
    
    row = VGA_HEIGHT - 1;
}

void Console::putchar(char c) {
    if (c == '\n') {
        column = 0;
        if (++row == VGA_HEIGHT) {
            scroll();
        }
    } else {
        const size_t index = row * VGA_WIDTH + column;
        VGA_MEMORY[index] = make_vga_entry(c, color);
        
        if (++column == VGA_WIDTH) {
            column = 0;
            if (++row == VGA_HEIGHT) {
                scroll();
            }
        }
    }
    
    update_cursor();
}

void Console::write(const char* str) {
    while (*str) {
        putchar(*str++);
    }
}

void Console::update_cursor() {
    uint16_t pos = row * VGA_WIDTH + column;
    IO::outb(0x3D4, 0x0F);
    IO::outb(0x3D5, (uint8_t)(pos & 0xFF));
    IO::outb(0x3D4, 0x0E);
    IO::outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void print_number(uint64_t num, int base, bool uppercase, int width, char pad) {
    char buffer[65];
    int i = 0;
    
    if (num == 0) {
        buffer[i++] = '0';
    } else {
        const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
        while (num > 0) {
            buffer[i++] = digits[num % base];
            num /= base;
        }
    }
    
    while (i < width) {
        buffer[i++] = pad;
    }
    
    while (i > 0) {
        Console::putchar(buffer[--i]);
    }
}

static void print_signed(int64_t num, int width, char pad) {
    if (num < 0) {
        Console::putchar('-');
        num = -num;
        if (width > 0) width--;
    }
    print_number((uint64_t)num, 10, false, width, pad);
}

void Console::printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    while (*format) {
        if (*format == '%') {
            format++;
            
            int width = 0;
            char pad = ' ';
            if (*format == '0') {
                pad = '0';
                format++;
            }
            while (*format >= '0' && *format <= '9') {
                width = width * 10 + (*format - '0');
                format++;
            }
            
            bool is_long = false;
            bool is_long_long = false;
            if (*format == 'l') {
                is_long = true;
                format++;
                if (*format == 'l') {
                    is_long_long = true;
                    format++;
                }
            }
            
            switch (*format) {
                case 'd':
                case 'i': {
                    int64_t val;
                    if (is_long_long) {
                        val = va_arg(args, int64_t);
                    } else if (is_long) {
                        val = va_arg(args, long);
                    } else {
                        val = va_arg(args, int);
                    }
                    print_signed(val, width, pad);
                    break;
                }
                case 'u': {
                    uint64_t val;
                    if (is_long_long) {
                        val = va_arg(args, uint64_t);
                    } else if (is_long) {
                        val = va_arg(args, unsigned long);
                    } else {
                        val = va_arg(args, unsigned int);
                    }
                    print_number(val, 10, false, width, pad);
                    break;
                }
                case 'x':
                    print_number(va_arg(args, uint64_t), 16, false, width, pad);
                    break;
                case 'X':
                    print_number(va_arg(args, uint64_t), 16, true, width, pad);
                    break;
                case 'p':
                    write("0x");
                    print_number((uint64_t)va_arg(args, void*), 16, false, 16, '0');
                    break;
                case 'c':
                    putchar((char)va_arg(args, int));
                    break;
                case 's': {
                    const char* str = va_arg(args, const char*);
                    write(str ? str : "(null)");
                    break;
                }
                case '%':
                    putchar('%');
                    break;
                default:
                    putchar('%');
                    putchar(*format);
                    break;
            }
        } else {
            putchar(*format);
        }
        format++;
    }
    
    va_end(args);
}

}