// terminal.h - Declaraciones del driver VGA
#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stddef.h>

// VGA text mode
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
    VGA_COLOR_YELLOW = 16,
} vga_color;

#define SHADOW_CHAR 0xB0  // Carácter ░ en página de códigos 437
// Caracteres de dibujo de caja (Box-drawing characters)
#define BOX_SINGLE_TOP_LEFT      0xDA    // ┌
#define BOX_SINGLE_TOP_RIGHT     0xBF    // ┐
#define BOX_SINGLE_BOTTOM_LEFT   0xC0    // └
#define BOX_SINGLE_BOTTOM_RIGHT  0xD9    // ┘
#define BOX_SINGLE_HORIZONTAL    0xC4    // ─
#define BOX_SINGLE_VERTICAL      0xB3    // │

#define BOX_DOUBLE_TOP_LEFT      0xC9    // ╔
#define BOX_DOUBLE_TOP_RIGHT     0xBB    // ╗
#define BOX_DOUBLE_BOTTOM_LEFT   0xC8    // ╚
#define BOX_DOUBLE_BOTTOM_RIGHT  0xBC    // ╝
#define BOX_DOUBLE_HORIZONTAL    0xCD    // ═
#define BOX_DOUBLE_VERTICAL      0xBA    // ║

#define BOX_CROSS_SINGLE         0xC5    // ┼
#define BOX_CROSS_DOUBLE         0xCE    // ╬

#define BOX_SINGLE_TO_DOUBLE_RIGHT 0xB7  // ╡
#define BOX_SINGLE_TO_DOUBLE_LEFT  0xB6  // ╢
#define BOX_DOUBLE_TO_SINGLE_RIGHT 0xB5  // ╖
#define BOX_DOUBLE_TO_SINGLE_LEFT  0xB4  // ╕

static inline uint8_t vga_entry_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t) c | (uint16_t) color << 8;
}

// Funciones del terminal
void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void terminal_scroll(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void print_hex(uint64_t value);
void print_dec(uint64_t value);

// Funciones para dibujar sombreado
void terminal_draw_shadow_char(size_t x, size_t y, vga_color fg_color, vga_color bg_color);
void terminal_draw_shadow_pattern(size_t start_x, size_t start_y, size_t width, size_t height, vga_color fg_color, vga_color bg_color);

// Funciones para dibujar cajas
void terminal_draw_box_char(size_t x, size_t y, uint8_t box_char, vga_color fg_color, vga_color bg_color);
void terminal_draw_single_box(size_t x, size_t y, size_t width, size_t height, vga_color fg_color, vga_color bg_color);
void terminal_draw_double_box(size_t x, size_t y, size_t width, size_t height, vga_color fg_color, vga_color bg_color);
void terminal_draw_double_horizontal_line(size_t x, size_t y, size_t length, vga_color fg_color, vga_color bg_color);
void terminal_draw_double_vertical_line(size_t x, size_t y, size_t length, vga_color fg_color, vga_color bg_color);

#endif