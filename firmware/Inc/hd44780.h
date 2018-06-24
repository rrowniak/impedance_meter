#ifndef __hd44780_h
#define __hd44780_h

#include "main.h"
#include <stdint.h>

#define LCD_CTRL_PORT LCD_PIN_E_GPIO_Port
#define LCD_RS LCD_PIN_RS_Pin
#define LCD_RW LCD_PIN_RW_Pin
#define LCD_EN LCD_PIN_E_Pin

#define LCD_DATA_PORT LCD_PIN_D4_GPIO_Port
#define LCD_D4 LCD_PIN_D4_Pin
#define LCD_D5 LCD_PIN_D5_Pin
#define LCD_D6 LCD_PIN_D6_Pin
#define LCD_D7 LCD_PIN_D7_Pin

#define LCD_COL_COUNT 16
#define LCD_ROW_COUNT 2

void lcd_init(void);

void lcd_command(uint8_t command);
void lcd_write(uint8_t value);

void lcd_on(void);
void lcd_off(void);

void lcd_clear(void);
void lcd_return_home(void);

void lcd_enable_blinking(void);
void lcd_disable_blinking(void);

void lcd_enable_cursor(void);
void lcd_disable_cursor(void);

void lcd_scroll_left(void);
void lcd_scroll_right(void);

void lcd_set_left_to_right(void);
void lcd_set_right_to_left(void);

void lcd_enable_autoscroll(void);
void lcd_disable_autoscroll(void);

void lcd_create_char(uint8_t location, uint8_t *charmap);

void lcd_set_cursor(uint8_t col, uint8_t row);

void lcd_puts(char *string);
void lcd_println(char *string, uint8_t line);
void lcd_printf(char *format, ...);
void lcd_printfln(char *format, ...);

#endif // __hd44780_h
