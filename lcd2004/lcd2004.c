// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.



#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "i2c_bus.h"
//#include "pcf8574.h"
#include "lcd2004.h"


#ifdef CONFIG_COMPONENT_LCD2004


static const char* TAG = "LCD";

#define PARAM_LCD "lcd"
#define PARAM_LCD_ADDR "addr"
#define PARAM_LCD_STATE "st"
#define PARAM_LCD_BL "bl"       // backlight

static lcd2004_conf_t lcd2004;

const uint8_t lcd_char_degree[8] =      // кодируем символ градуса
{
  0b00111,
  0b00101,
  0b00111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
}; 

static lcd2004_line_addr_t lcd_line[4] = {LCD2004_LINE_1, LCD2004_LINE_2, LCD2004_LINE_3, LCD2004_LINE_4};

static esp_err_t lcd2004_i2c_write_byte(uint8_t val);




static void lcd2004_send_half_byte(uint8_t nibble, uint8_t mode);
static void lcd2004_send_byte(uint8_t cmd, uint8_t mode);

void lcd2004_init();
void lcd2004_set_cursor();
void lcd2004_set_text();
void lcd2004_set_backlight();
void lcd2004_clear();

static const uint8_t line_addr[] = { LCD2004_LINE_1, LCD2004_LINE_2, LCD2004_LINE_3, LCD2004_LINE_4 };

static esp_err_t lcd2004_i2c_write_byte(uint8_t val)
{
    static uint8_t retry = 0;
    if ( xSemaphoreI2C == NULL ) return ESP_FAIL;
retr:
    if ( xSemaphoreTake( xSemaphoreI2C, LCD_I2C_SEMAPHORE_WAIT ) == pdFALSE ) {
        ESP_LOGE(TAG, "%s Semaphore taken error. Retry %d", __func__, retry);
        if ( retry > LCD_I2C_SEND_RETRY_COUNT ) return ESP_FAIL;
        retry++;
        goto retr;
        //return ESP_FAIL;
    }
    retry = 0;
    esp_err_t err = i2c_send_command(lcd2004.addr, val);
    xSemaphoreGive( xSemaphoreI2C );
    return err;
}

static void lcd2004_send_i2c(uint8_t nibble, uint8_t mode, uint8_t enable)
{
    uint8_t i2c_data = nibble << 4;
    if ( mode == RS_MODE_DATA ) 
        i2c_data |= RS;
    else 
        i2c_data &= ~RS;

    if ( enable ) 
        i2c_data |= EN;
    else
        i2c_data &= ~EN;

    if ( lcd2004.backlight == LCD2004_BACKLIGHT_ON ) 
    //if ( lcd2004.state == LCD2004_STATE_ON ) 
        i2c_data |= BL;
    else
        i2c_data &= ~BL;    

    lcd2004_i2c_write_byte( i2c_data );   
    os_delay_us( 50 ); 
}

static void lcd2004_send_half_byte(uint8_t nibble, uint8_t mode)
{  
    // EN = 1, 
    lcd2004_send_i2c( nibble, mode, 1);
    i2c_master_wait(1);

    // EN = 0
    lcd2004_send_i2c( nibble, mode, 0);
    i2c_master_wait(1);  // >37us
}

static void lcd2004_send_byte(uint8_t cmd, uint8_t mode)
{
    lcd2004_send_half_byte( cmd >> 4, mode);
    lcd2004_send_half_byte( cmd, mode);
}

static void lcd2004_send_command(uint8_t cmd)
{
    lcd2004_send_byte(cmd, RS_MODE_CMD);
}


void lcd2004_load_cfg(lcd2004_conf_t *cfg)
{
    if ( nvs_param_u8_load(PARAM_LCD, PARAM_LCD_ADDR, &cfg->addr) != ESP_OK ) {
        cfg->addr = LCD2004_ADDR_DEFAULT;
    }

    if ( nvs_param_u8_load(PARAM_LCD, PARAM_LCD_STATE, &cfg->state) != ESP_OK ) {
        cfg->state = LCD2004_STATE_ON;
    }

    if ( nvs_param_u8_load(PARAM_LCD, PARAM_LCD_BL, &cfg->backlight) != ESP_OK ) {
        cfg->backlight = LCD2004_BACKLIGHT_ON;
    }
        ESP_LOGI(TAG, "loaded addr 0x%02X", cfg->addr);
        ESP_LOGI(TAG, "loaded state %d", cfg->state);
        ESP_LOGI(TAG, "loaded backlight %d", cfg->backlight);    
}

void lcd2004_save_cfg(const lcd2004_conf_t *cfg)
{
        ESP_LOGI(TAG, "save addr 0x%02X", cfg->addr);
        ESP_LOGI(TAG, "save state %d", cfg->state);
        ESP_LOGI(TAG, "save backlight %d", cfg->backlight);

    nvs_param_u8_save(PARAM_LCD, PARAM_LCD_ADDR, cfg->addr);
    nvs_param_u8_save(PARAM_LCD, PARAM_LCD_STATE, cfg->state);
    nvs_param_u8_save(PARAM_LCD, PARAM_LCD_BL, cfg->backlight);
}

void lcd2004_get_cfg(lcd2004_conf_t *cfg)
{
    memcpy(cfg, &lcd2004, sizeof(lcd2004_conf_t));
}
//void lcd2004_init(uint8_t addr, uint8_t cols, uint8_t rows)
void lcd2004_init()
{
    
    lcd2004_load_cfg(&lcd2004);

    ESP_LOGI(TAG, "ADD 0x%02X", lcd2004.addr);
    ESP_LOGI(TAG, "state %d", lcd2004.state);
    ESP_LOGI(TAG, "backlight %d", lcd2004.backlight);

    //lcd2004.addr = addr;
    lcd2004.cols = 20; //cols;
    lcd2004.rows = 4; //rows;
    lcd2004.control_flag = 0;
    lcd2004.font_size = LCD2004_FONT_5X8;
    //lcd2004.backlight = LCD2004_BACKLIGHT_ON;   // loaded from nvs
    lcd2004.mode = 0;
    //lcd2004.state = LCD2004_STATE_OFF;            // loaded from nvs

    if ( xSemaphoreLCD2004 != NULL ) vSemaphoreDelete( xSemaphoreLCD2004 );
    xSemaphoreLCD2004 = xSemaphoreCreateMutex();

    lcd2004.i2c_bus_handle = i2c_bus_init();


    if ( xSemaphoreLCD2004 == NULL ) return;
    if( xSemaphoreTake( xSemaphoreLCD2004, I2C_SEMAPHORE_WAIT ) == pdFALSE ) return;

    lcd2004_i2c_write_byte( 0x00 );
    vTaskDelay( 500 / portTICK_RATE_MS);

    lcd2004_send_half_byte( 0x03, RS_MODE_CMD );
    i2c_master_wait( 4500 );

    lcd2004_send_half_byte( 0x03, RS_MODE_CMD ); 
    i2c_master_wait( 4500 );

    lcd2004_send_half_byte( 0x03, RS_MODE_CMD ); 
    i2c_master_wait( 150 );

    lcd2004_send_half_byte( 0x02, RS_MODE_CMD ); 

    lcd2004_send_command( LCD_CMD_FUNCTION_SET | LCD_2LINE | lcd2004.font_size);

    lcd2004_send_command( LCD_CMD_CONTROL);
    i2c_master_wait( 1000 );

    lcd2004_send_command( LCD_CMD_CLEAR );
    i2c_master_wait( 2000 );

    lcd2004.mode = LCD_CMD_ENTRY_LEFT | LCD_CMD_ENTRY_SHIFT_OFF;
    i2c_master_wait(1000);    

     lcd2004.control_flag |= LCD_CMD_DISPLAY_OFF | LCD_CMD_UNDERLINE_CURSOR_OFF | LCD_CMD_BLINK_CURSOR_OFF;

    if ( lcd2004.state == LCD2004_STATE_ON )
    {
        lcd2004.control_flag |= LCD_CMD_DISPLAY_ON;
    } 
    else 
    {
        lcd2004.control_flag = LCD_CMD_DISPLAY_OFF ;
    }   

        //lcd2004.control_flag = LCD_CMD_DISPLAY_ON | LCD_CMD_UNDERLINE_CURSOR_OFF | LCD_CMD_BLINK_CURSOR_OFF;
    lcd2004_send_command( LCD_CMD_CONTROL | lcd2004.control_flag); 

    //lcd2004.state = LCD2004_STATE_ON;

    xSemaphoreGive( xSemaphoreLCD2004 );

    lcd2004_home();
    lcd2004_clear();

    lcd2004_backlight(lcd2004.backlight);

    //lcd2004_send_command( LCD_CMD_DDRAM_ADDR_SET);
    //i2c_master_wait(2000);
}

void lcd2004_backlight(lcd2004_backlight_t state)
{
    ESP_LOGI(TAG, "%s", __func__ );
    lcd2004.backlight = state;

    lcd2004_send_command( LCD_CMD_CONTROL | lcd2004.control_flag);
}

lcd2004_backlight_t lcd2004_backlight_state()
{
    return lcd2004.backlight;
}

void lcd2004_set_state(lcd2004_state_t state)
{
    ESP_LOGI(TAG, "%s", __func__ );
    lcd2004.state = state;
    if ( state == LCD2004_STATE_ON )
    {
        lcd2004.backlight = LCD2004_BACKLIGHT_ON;
        lcd2004.control_flag |= LCD_CMD_DISPLAY_ON | LCD_CMD_UNDERLINE_CURSOR_OFF | LCD_CMD_BLINK_CURSOR_OFF;
    } 
    else 
    {
        lcd2004.backlight = LCD2004_BACKLIGHT_OFF;
        lcd2004.control_flag = LCD_CMD_DISPLAY_OFF ;
    }    
    lcd2004_send_command( LCD_CMD_CONTROL | lcd2004.control_flag);
}

lcd2004_state_t lcd2004_state()
{
    return lcd2004.state;
}



void lcd2004_cursor_show(uint8_t val)
{
    if ( val )
        lcd2004.control_flag |= LCD_CMD_UNDERLINE_CURSOR_ON;
    else
        lcd2004.control_flag &= ~ LCD_CMD_UNDERLINE_CURSOR_ON;
    
    lcd2004_send_command( LCD_CMD_CONTROL | lcd2004.control_flag);
}

void lcd2004_cursor_blink(uint8_t val)
{
    if ( val )
        lcd2004.control_flag |= LCD_CMD_BLINK_CURSOR_ON;
    else
        lcd2004.control_flag &= ~ LCD_CMD_BLINK_CURSOR_ON;

    lcd2004_send_command( LCD_CMD_CONTROL | lcd2004.control_flag);
}

void lcd2004_set_cursor_position(uint8_t col, uint8_t row)
{
    uint8_t val = LCD_CMD_DDRAM_ADDR_SET | (col-1) | lcd_line[row-1];
    lcd2004_send_command ( val );   
}

static void lcd2004_print_char(char ch)
{
    lcd2004_send_byte( ch, RS_MODE_DATA);
}

void lcd2004_print_string(char *str)
{
    while (*str)
    {
        lcd2004_print_char(*str);
        str++;
    }
}


void lcd2004_print_string_at_pos(uint8_t col, uint8_t row, char *str)
{
    lcd2004_set_cursor_position( col, row );
    lcd2004_print_string(str);
}

void lcd2004_clear()
{
    lcd2004_send_command( LCD_CMD_CLEAR );//уберем мусор LCD2004_CMD_CLEAR
    i2c_master_wait( 2000 );
}

void lcd2004_home()
{
    lcd2004_send_command( LCD_CMD_RETURN_HOME);//курсор на место
    i2c_master_wait(2000); 
}

void lcd2004_print(uint8_t line, const char *str)
{
    uint8_t len = strlen(str);
    char *s = (char *) calloc( LCD_LINE_LENGTH + 1, sizeof(char*));
    memcpy(s, str, LCD_LINE_LENGTH);
    memset(s+len, 0x20, LCD_LINE_LENGTH-len);

    lcd2004_set_cursor_position( 1, line);
    lcd2004_print_string( s );
    free(s);
    
}

void lcd2004_progress(uint8_t line, uint8_t val, uint8_t blink)
{
    char *s = (char *) calloc( 10 + 1, sizeof(char*));
    sprintf(s, "%3d%%", val);
    lcd2004_progress_text(line, s, val, blink);
    free(s);
}

void lcd2004_progress_text(uint8_t line, const char *str, uint8_t val, uint8_t blink)
{
    // под прогессбар отдаем всю строку
    char *s = (char *) calloc( LCD_LINE_LENGTH + 1, sizeof(char*));
    uint8_t len = strlen(str) ;
    uint8_t progress_len = LCD_LINE_LENGTH - len;
    uint8_t progress = ( val ) * progress_len / 100;
    
    // show progress bar
    memset(s, 0x20, LCD_LINE_LENGTH); // заполнили пробелами
    memset(s, 0xFF, progress); // заполнили прогресс
    memcpy(s+progress_len, str, len);
    lcd2004_print(line, s);    
    
    if ( blink > 0 )
    {
        lcd2004_set_cursor_position( progress+1, line);
        lcd2004_cursor_blink( progress < progress_len );
    }

    free(s);
}

void lcd2004_test_task_cb(void *arg)
{

    while (1)
    {

        for ( uint8_t i = 1; i <= 100; i++)
        {
            if ( xSemaphoreLCD2004 != NULL && xSemaphoreTake( xSemaphoreLCD2004, I2C_SEMAPHORE_WAIT ) == pdTRUE ) 
            {
                lcd2004_progress(4, i, 0);
                xSemaphoreGive( xSemaphoreLCD2004 );
            }
            
            vTaskDelay(  2000 / portTICK_RATE_MS ); 
        }
        
        vTaskDelay(  10000 / portTICK_RATE_MS );   
    }

    vTaskDelete( NULL );
}
void lcd2004_test_task()
{
        xTaskCreate(lcd2004_test_task_cb, "lcd2004_test", 2048, NULL, 15, NULL);
    
}
#endif
