#ifndef CORE_CONSOLE_H
#define CORE_CONSOLE_H

#include <kernel/types.h>

namespace Core {

class Console {
public:
    enum class Color : uint8_t {
        BLACK = 0, BLUE = 1, GREEN = 2, CYAN = 3,
        RED = 4, MAGENTA = 5, BROWN = 6, LIGHT_GRAY = 7,
        DARK_GRAY = 8, LIGHT_BLUE = 9, LIGHT_GREEN = 10,
        LIGHT_CYAN = 11, LIGHT_RED = 12, LIGHT_MAGENTA = 13,
        YELLOW = 14, WHITE = 15
    };

    static void initialize();
    static void clear();
    static void putchar(char c);
    static void write(const char* str);
    static void printf(const char* format, ...);
    static void set_color(Color fg, Color bg);

private:
    static constexpr size_t VGA_WIDTH = 80;
    static constexpr size_t VGA_HEIGHT = 25;
    static constexpr uint16_t* VGA_MEMORY = (uint16_t*)0xFFFFFFFF800B8000;
    
    static size_t row;
    static size_t column;
    static uint8_t color;
    
    static uint8_t make_color(Color fg, Color bg);
    static uint16_t make_vga_entry(char c, uint8_t color);
    static void scroll();
    static void update_cursor();
};

}

#endif