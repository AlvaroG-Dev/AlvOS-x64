// terminal.c - Driver VGA mejorado con utilidades adicionales
#include "terminal.h"
#include "string.h"

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) VGA_MEMORY;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }
    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_scroll();
        return;
    }
    
    if (c == '\b') {  // Backspace
        if (terminal_column > 0) {
            terminal_column--;
            terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        }
        return;
    }
    
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT)
            terminal_scroll();
    }
}

// Dibujar carácter de sombreado ░ en posición específica
void terminal_draw_shadow_char(size_t x, size_t y, vga_color fg_color, vga_color bg_color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    uint8_t color = vga_entry_color(fg_color, bg_color);
    terminal_putentryat(SHADOW_CHAR, color, x, y);
}

// Dibujar un patrón de sombreado
void terminal_draw_shadow_pattern(size_t start_x, size_t start_y, size_t width, size_t height, vga_color fg_color, vga_color bg_color) {
    for (size_t y = start_y; y < start_y + height && y < VGA_HEIGHT; y++) {
        for (size_t x = start_x; x < start_x + width && x < VGA_WIDTH; x++) {
            terminal_draw_shadow_char(x, y, fg_color, bg_color);
        }
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

// Imprimir número en hexadecimal
void print_hex(uint64_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[17];
    buffer[16] = '\0';
    
    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_chars[value & 0xF];
        value >>= 4;
    }
    
    terminal_writestring(buffer);
}

// Imprimir número decimal
void print_dec(uint64_t value) {
    if (value == 0) {
        terminal_putchar('0');
        return;
    }
    
    char buffer[21];  // Suficiente para uint64_t
    int i = 0;
    
    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Imprimir al revés
    while (i > 0) {
        terminal_putchar(buffer[--i]);
    }
}

// Dibujar un carácter de caja en posición específica
void terminal_draw_box_char(size_t x, size_t y, uint8_t box_char, vga_color fg_color, vga_color bg_color) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    uint8_t color = vga_entry_color(fg_color, bg_color);
    terminal_putentryat(box_char, color, x, y);
}

// Dibujar una caja simple
void terminal_draw_single_box(size_t x, size_t y, size_t width, size_t height, vga_color fg_color, vga_color bg_color) {
    // Esquinas
    terminal_draw_box_char(x, y, BOX_SINGLE_TOP_LEFT, fg_color, bg_color);
    terminal_draw_box_char(x + width - 1, y, BOX_SINGLE_TOP_RIGHT, fg_color, bg_color);
    terminal_draw_box_char(x, y + height - 1, BOX_SINGLE_BOTTOM_LEFT, fg_color, bg_color);
    terminal_draw_box_char(x + width - 1, y + height - 1, BOX_SINGLE_BOTTOM_RIGHT, fg_color, bg_color);
    
    // Bordes horizontales
    for (size_t i = x + 1; i < x + width - 1; i++) {
        terminal_draw_box_char(i, y, BOX_SINGLE_HORIZONTAL, fg_color, bg_color);
        terminal_draw_box_char(i, y + height - 1, BOX_SINGLE_HORIZONTAL, fg_color, bg_color);
    }
    
    // Bordes verticales
    for (size_t j = y + 1; j < y + height - 1; j++) {
        terminal_draw_box_char(x, j, BOX_SINGLE_VERTICAL, fg_color, bg_color);
        terminal_draw_box_char(x + width - 1, j, BOX_SINGLE_VERTICAL, fg_color, bg_color);
    }
}

// Dibujar una caja doble (como la que muestras)
void terminal_draw_double_box(size_t x, size_t y, size_t width, size_t height, vga_color fg_color, vga_color bg_color) {
    // Esquinas
    terminal_draw_box_char(x, y, BOX_DOUBLE_TOP_LEFT, fg_color, bg_color);
    terminal_draw_box_char(x + width - 1, y, BOX_DOUBLE_TOP_RIGHT, fg_color, bg_color);
    terminal_draw_box_char(x, y + height - 1, BOX_DOUBLE_BOTTOM_LEFT, fg_color, bg_color);
    terminal_draw_box_char(x + width - 1, y + height - 1, BOX_DOUBLE_BOTTOM_RIGHT, fg_color, bg_color);
    
    // Bordes horizontales
    for (size_t i = x + 1; i < x + width - 1; i++) {
        terminal_draw_box_char(i, y, BOX_DOUBLE_HORIZONTAL, fg_color, bg_color);
        terminal_draw_box_char(i, y + height - 1, BOX_DOUBLE_HORIZONTAL, fg_color, bg_color);
    }
    
    // Bordes verticales
    for (size_t j = y + 1; j < y + height - 1; j++) {
        terminal_draw_box_char(x, j, BOX_DOUBLE_VERTICAL, fg_color, bg_color);
        terminal_draw_box_char(x + width - 1, j, BOX_DOUBLE_VERTICAL, fg_color, bg_color);
    }
}

// Dibujar línea horizontal doble
void terminal_draw_double_horizontal_line(size_t x, size_t y, size_t length, vga_color fg_color, vga_color bg_color) {
    for (size_t i = 0; i < length && (x + i) < VGA_WIDTH; i++) {
        terminal_draw_box_char(x + i, y, BOX_DOUBLE_HORIZONTAL, fg_color, bg_color);
    }
}

// Dibujar línea vertical doble
void terminal_draw_double_vertical_line(size_t x, size_t y, size_t length, vga_color fg_color, vga_color bg_color) {
    for (size_t i = 0; i < length && (y + i) < VGA_HEIGHT; i++) {
        terminal_draw_box_char(x, y + i, BOX_DOUBLE_VERTICAL, fg_color, bg_color);
    }
}