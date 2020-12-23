#pragma once

#ifndef _LCD2004_H_
#define _LCD2004_H_

#include <stdio.h>
#include <stdlib.h>

#include "esp_err.h"
#include "driver/i2c.h"
#include "nvsparam.h"
#include "i2c_bus.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "utils.h"

#ifdef CONFIG_COMPONENT_LCD2004
//#define LCD2004_ADDR_DEFAULT 0x3F
#define LCD2004_ADDR_DEFAULT CONFIG_COMPONENT_LCD2004_ADDR

#define RS ( 1 << 0 ) // 0x01
#define RW ( 1 << 1 ) // 0x02
#define EN ( 1 << 2 ) // 0x04
#define BL ( 1 << 3 ) // 0x08

#define RS_MODE_CMD  0
#define RS_MODE_DATA 1

/*
   LCD_INSTRUCTION_WRITE instructions
   NOTE: all instructions formated as DB7=(set DDRAM), DB6=(set CGRAM), DB5=(function set), DB4=(cursor shift), DB3=(disp. control), DB2=(entry mode set), DB1, DB0
*/
#define LCD_CMD_CLEAR               0x01        // clears display & move cursor to home position
#define LCD_CMD_RETURN_HOME         0x02        // moves cursor position to home position
#define LCD_CMD_ENTRY_MODE_SET      0x04        // sets cursor type, text direction (I/D) & display shift direction (S)
#define LCD_CMD_CONTROL             0x08        // sets display on/off (D), cursor on/off (C) & cursor blinking on/off (B)
#define LCD_CMD_SHIFT               0x10        // cursor or display/text shifts without changing DDRAM contents
#define LCD_CMD_FUNCTION_SET        0x20        // sets data length (DL), number of lines (N) & font size (F)
#define LCD_CMD_CGRAM_ADDR_SET      0x40        // sets CGRAM address. CGRAM data is sent & received after this setting
#define LCD_CMD_DDRAM_ADDR_SET      0x80        // sets DDRAM address/cursor position

/*
   LCD_ENTRY_MODE_SET controls
   NOTE: all controls formated as DB7, DB6, DB5, DB4, DB3, DB2, DB1=(I/D), DB0=(S)
*/
#define LCD_CMD_ENTRY_RIGHT          0x00       // sets text direction decrement/"right to left" (I/D)
#define LCD_CMD_ENTRY_LEFT           0x02       // sets text direction increment/"left to right" (I/D)
#define LCD_CMD_ENTRY_SHIFT_ON       0x01       // text shifts when byte written & cursot stays (S)
#define LCD_CMD_ENTRY_SHIFT_OFF      0x00       // text stays & cursor moves when byte written  (S)

/*
   LCD_DISPLAY_CONTROL controls
   NOTE: all controls formated as DB7, DB6, DB5, DB4, DB3, DB2=(D), DB1=(C), DB0=(B)
*/
#define LCD_CMD_DISPLAY_ON           0x04        // turns display ON/retrive text (D)
#define LCD_CMD_DISPLAY_OFF          0x00        // turns display OFF/clears text (D)
#define LCD_CMD_UNDERLINE_CURSOR_ON  0x02        // turns ON  underline cursor (C)
#define LCD_CMD_UNDERLINE_CURSOR_OFF 0x00        // turns OFF underline cursor (C)
#define LCD_CMD_BLINK_CURSOR_ON      0x01        // turns ON  blinking  cursor (B)
#define LCD_CMD_BLINK_CURSOR_OFF     0x00        // turns OFF blinking  cursor (B)

#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00

#define LCD_LINE_LENGTH 20

#define LCD_I2C_SEMAPHORE_WAIT_TIME_MS             200
#define LCD_I2C_SEMAPHORE_WAIT  (I2C_SEMAPHORE_WAIT_TIME_MS /  portTICK_RATE_MS)
#define LCD_I2C_SEND_RETRY_COUNT 30

extern const uint8_t lcd_char_degree[8];

SemaphoreHandle_t xSemaphoreLCD2004;

typedef enum lcd2004_cmd_state {
    LCD2004_CMD_STATE_OFF,
    LCD2004_CMD_STATE_ON
} lcd2004_cmd_state_t;

typedef enum lcd2004_font_size {
    LCD2004_FONT_5X8 = 0,
    LCD2004_FONT_5X10 = 4    
} lcd2004_font_size_t;

typedef enum lcd2004_backlight {
    LCD2004_BACKLIGHT_OFF,
    LCD2004_BACKLIGHT_ON
} lcd2004_backlight_t;

typedef enum lcd2004_state {
    LCD2004_STATE_OFF,
    LCD2004_STATE_ON
} lcd2004_state_t;

typedef struct lcd2004_conf {
    uint8_t addr;
    uint8_t cols;
    uint8_t rows;
    uint8_t control_flag;
    uint8_t mode;
    uint8_t cursor_pos;
    lcd2004_state_t state;
    lcd2004_font_size_t font_size;
    lcd2004_backlight_t backlight;
    i2c_bus_handle_t i2c_bus_handle;
} lcd2004_conf_t;

typedef enum lcd2004_line_addr {
    LCD2004_LINE_1 = 0x00,      // 1.0..1.19
    LCD2004_LINE_2 = 0x40,      // 2.0..2.19
    LCD2004_LINE_3 = 0x14,      // 1.20..1.39 = 3.0..3.19
    LCD2004_LINE_4 = 0x54
} lcd2004_line_addr_t;


void lcd2004_load_cfg(lcd2004_conf_t *cfg);
void lcd2004_get_cfg(lcd2004_conf_t *cfg);
void lcd2004_save_cfg(const lcd2004_conf_t *cfg);

void lcd2004_init();
void lcd2004_clear();
void lcd2004_home();

void lcd2004_set_state(lcd2004_state_t state);
lcd2004_state_t lcd2004_state();

void lcd2004_backlight(lcd2004_backlight_t state);
lcd2004_backlight_t lcd2004_backlight_state();

void lcd2004_cursor_show(uint8_t val);
void lcd2004_cursor_blink(uint8_t val);

void lcd2004_set_cursor_position(uint8_t col, uint8_t row);
void lcd2004_print_string(char *str);
void lcd2004_print_string_at_pos(uint8_t col, uint8_t row, char *str);

void lcd2004_print(uint8_t line, const char *str);
void lcd2004_progress(uint8_t line, uint8_t val, uint8_t blink);
void lcd2004_progress_text(uint8_t line, const char *str, uint8_t val, uint8_t blink);


void lcd2004_test_task();

#endif //CONFIG_COMPONENT_LCD2004
#endif