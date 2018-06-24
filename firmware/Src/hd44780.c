#include "hd44780.h"

#include "stm32f1xx_hal.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LCD_CLEARDISPLAY   0x01
#define LCD_RETURNHOME     0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT    0x10
#define LCD_FUNCTIONSET    0x20
#define LCD_SETCGRAMADDR   0x40
#define LCD_SETDDRAMADDR   0x80

#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

#define LCD_DISPLAYON  0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON   0x02
#define LCD_CURSOROFF  0x00
#define LCD_BLINKON    0x01
#define LCD_BLINKOFF   0x00

#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE  0x00
#define LCD_MOVERIGHT   0x04
#define LCD_MOVELEFT    0x00

#define LCD_LCD_D4DE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE    0x08
#define LCD_1LINE    0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS  0x00

static inline void _lcd_bus_wait(void)
{
    for (int i = 0; i < 1000; ++i) {
        asm volatile("NOP"); // ~14 ns
    }
}

static void lcd_send(uint8_t value, uint8_t mode);
static void lcd_write_nibble(uint8_t nibble);

static uint8_t lcd_displayparams;
static char lcd_buffer[LCD_COL_COUNT + 1];

void lcd_command(uint8_t command) {
    lcd_send(command, 0);
}

void lcd_write(uint8_t value) {
    lcd_send(value, 1);
}

void lcd_send(uint8_t value, uint8_t mode) {
    GPIO_PinState pin_state;

    if (mode) {
        pin_state = GPIO_PIN_SET;
    } else {
        pin_state = GPIO_PIN_RESET;
    }

    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS, pin_state);
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RW, GPIO_PIN_RESET);

    lcd_write_nibble(value >> 4);
    lcd_write_nibble(value);
}

void lcd_write_nibble(uint8_t nibble) {
    static int d_pins[] = {LCD_D4, LCD_D5, LCD_D6, LCD_D7};

    for (int i = 0; i < 4; ++i) {
        if (nibble & 0x0f & (1 << i)) {
            HAL_GPIO_WritePin(LCD_DATA_PORT, d_pins[i], GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(LCD_DATA_PORT, d_pins[i], GPIO_PIN_RESET);
        }
    }

    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_EN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_EN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_EN, GPIO_PIN_RESET);

    _lcd_bus_wait();
    // HAL_Delay(1);
}

void lcd_init(void) {

    // Wait for LCD to become ready (docs say 15ms+)
    HAL_Delay(15);
    //HAL_Delay(150);
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_EN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RW, GPIO_PIN_RESET);

    HAL_Delay(4.1);

    lcd_write_nibble(0x03); // Switch to 4 bit mode
    HAL_Delay(4.1);

    lcd_write_nibble(0x03); // 2nd time
    HAL_Delay(4.1);

    lcd_write_nibble(0x03); // 3rd time
    HAL_Delay(4.1);

    lcd_write_nibble(0x02); // Set 8-bit mode (?)

    lcd_command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);

    lcd_displayparams = LCD_CURSOROFF | LCD_BLINKOFF;
    lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_on(void)
{
    lcd_displayparams |= LCD_DISPLAYON;
    lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_off(void)
{
    lcd_displayparams &= ~LCD_DISPLAYON;
    lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_clear(void)
{
    lcd_command(LCD_CLEARDISPLAY);
    HAL_Delay(2);
}

void lcd_return_home(void)
{
    lcd_command(LCD_RETURNHOME);
    HAL_Delay(2);
}

void lcd_enable_blinking(void)
{
    lcd_displayparams |= LCD_BLINKON;
    lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_disable_blinking(void)
{
    lcd_displayparams &= ~LCD_BLINKON;
    lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_enable_cursor(void)
{
    lcd_displayparams |= LCD_CURSORON;
    lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_disable_cursor(void)
{
    lcd_displayparams &= ~LCD_CURSORON;
    lcd_command(LCD_DISPLAYCONTROL | lcd_displayparams);
}

void lcd_scroll_left(void)
{
    lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void lcd_scroll_right(void)
{
    lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void lcd_set_left_to_right(void)
{
    lcd_displayparams |= LCD_ENTRYLEFT;
    lcd_command(LCD_ENTRYMODESET | lcd_displayparams);
}

void lcd_set_right_to_left(void)
{
    lcd_displayparams &= ~LCD_ENTRYLEFT;
    lcd_command(LCD_ENTRYMODESET | lcd_displayparams);
}

void lcd_enable_autoscroll(void)
{
    lcd_displayparams |= LCD_ENTRYSHIFTINCREMENT;
    lcd_command(LCD_ENTRYMODESET | lcd_displayparams);
}

void lcd_disable_autoscroll(void)
{
    lcd_displayparams &= ~LCD_ENTRYSHIFTINCREMENT;
    lcd_command(LCD_ENTRYMODESET | lcd_displayparams);
}

void lcd_create_char(uint8_t location, uint8_t *charmap)
{
    lcd_command(LCD_SETCGRAMADDR | ((location & 0x7) << 3));
    for (int i = 0; i < 8; i++) {
        lcd_write(charmap[i]);
    }
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
    static uint8_t offsets[] = { 0x00, 0x40, 0x14, 0x54 };

    if (row > 1) {
        row = 1;
    }

    lcd_command(LCD_SETDDRAMADDR | (col + offsets[row]));
}

void lcd_puts(char *string)
{
    for (char *it = string; *it; it++) {
        lcd_write(*it);
    }
}

void lcd_println(char *string, uint8_t line)
{
    lcd_set_cursor(0, line);
    lcd_puts(string);
    size_t s = strlen(string);
    while (s++ < LCD_COL_COUNT) {
        lcd_write(' ');
    }
}

void lcd_printf(char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(lcd_buffer, LCD_COL_COUNT + 1, format, args);
    va_end(args);

    lcd_puts(lcd_buffer);
}

void lcd_printfln(char *format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(lcd_buffer, LCD_COL_COUNT + 1, format, args);
    va_end(args);

    lcd_puts(lcd_buffer);

    size_t s = strlen(lcd_buffer);
    while (s++ < LCD_COL_COUNT) {
        lcd_write(' ');
    }
}
