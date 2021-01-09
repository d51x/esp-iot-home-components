#include "button2.h"
#include "utils.h"

static const char *TAG = "button2";

#define BTN_GLITCH_TIME 50
#define BTN_SHORT_PRESS_TIMEOUT 300

static void init_gpio_button(button_t* btn)
{
    //gpio_install_isr_service(0);
    gpio_config_t gpio_conf;
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pin_bit_mask = (1ULL << btn->pin);
    if ( btn->level )
	{
        gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    }
	else
	{
        gpio_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    }
    gpio_config(&gpio_conf);
    //gpio_isr_handler_add(gpio_num, button_gpio_isr_handler, btn);   // обработчик прерывания, в качестве аргумента передается указатель на сам объект button
}


static void short_press_timer_cb(xTimerHandle tmr)
{
    button_t *btn = (button_t *)pvTimerGetTimerID(tmr); // rtos
    ESP_LOGW(TAG, "%s: btn = %p", __func__, btn);
    ESP_LOGI(TAG, "short count = %d", btn->short_press_count);

    uint8_t i = 0;
    for ( i = 0; i < btn->short_press_cb_cnt; i++ )
    {
        btn_cb_t cb = btn->short_press_cb[i];
        if ( cb.idx == btn->short_press_count )
        {
            if ( cb.cb  != NULL )
            {
                cb.cb(cb.args);
            }
            break;
        }
    }
}

static void reschedule_short_press_timer(button_t *btn)
{
    if ( btn->short_press_timer == NULL )
    {
        btn->short_press_timer = xTimerCreate("shortpress", BTN_SHORT_PRESS_TIMEOUT / portTICK_PERIOD_MS, pdFALSE, btn, short_press_timer_cb);
    } 
    
    if ( xTimerIsTimerActive(btn->short_press_timer) == pdTRUE )
    {
        xTimerStop(btn->short_press_timer, 0);
    }

    xTimerStart(btn->short_press_timer, 0);
}

static void btn_read_cb(xTimerHandle tmr) // rtos
{
    button_t *btn = (button_t *)pvTimerGetTimerID(tmr); //rtos
    int level = gpio_get_level( btn->pin );

    btn->ms = millis();

    if ( level == 0 && !btn->state && ( (btn->ms - btn->pressed_time) > BTN_GLITCH_TIME))
    {
        btn->state = 1;
        btn->pressed_time = btn->ms;
    }
    else if ( level == 1 && btn->state && ((btn->ms - btn->pressed_time) > BTN_GLITCH_TIME))
    {
        btn->hold_time = btn->ms - btn->pressed_time;
        btn->released_time = millis();

        btn->state = 0;

        if ( btn->short_press_count == 0 ) btn->short_press_count = 1;
        if ( btn->hold_time < BTN_SHORT_PRESS_TIMEOUT )
        {
            btn->wait_time = btn->pressed_time - btn->prev_pressed_time;
            if ( btn->wait_time < BTN_SHORT_PRESS_TIMEOUT )
            {
                btn->short_press_count++;
            } 
            else 
            {
                btn->short_press_count = 1;
            }        
            reschedule_short_press_timer(btn);
        } 
        else // ms - pressed_time > BTN_SHORT_PRESS_TIMEOUT
        {
            btn->short_press_count = 1;
            btn->long_press_time = btn->hold_time;

            if ( btn->long_press_time < 1000 ) {
                ESP_LOGI(TAG, "short pressed %d, hold time %d ms, wait = %d", btn->short_press_count, btn->hold_time, btn->wait_time);
                
                if ( btn->short_press_cb[0].cb != NULL  )
                {
                    btn->short_press_cb[0].cb(btn->short_press_cb[0].args );
                }
            } else {
                for ( uint8_t i = btn->long_press_cb_cnt; i > 0; i--)
                {
                    if ( btn->long_press_time >= btn->long_press_cb[i-1].timeout)
                    {
                        if ( btn->long_press_cb[i-1].cb != NULL )
                        {
                            btn->long_press_cb[i-1].cb( btn->long_press_cb[i-1].args );
                        }
                        break;
                    }
                }
            }
        }
        btn->prev_pressed_time = btn->pressed_time;
        btn->pressed_time = btn->ms;
        btn->ms = millis();
    }
}

button_t* create_button(gpio_num_t pin, button_active_t level)
{
	button_t *btn = (button_t *) malloc(sizeof(button_t));
	btn->level = level;
	btn->pin = pin;
	btn->state = 0;
	
	btn->short_press_count = 0;
	btn->long_press_time = 0;

	btn->pressed_time = 0;
	btn->prev_pressed_time = 0;
    btn->ms = 0;
    btn->released_time = 0;
    btn->hold_time = 0;
    btn->wait_time = 0;
	
    btn->short_press_timer = NULL;

    btn->short_press_cb = NULL;
    btn->short_press_cb_cnt = 0;

    btn->long_press_cb = NULL;
    btn->long_press_cb_cnt = 0;

    ESP_LOGW(TAG, "%s: pin = %d", __func__, btn->pin);
    
    init_gpio_button(btn);

    ESP_LOGW(TAG, "%s: btn = %p", __func__, btn);
    btn->read_timer = xTimerCreate("btn", 10 / portTICK_PERIOD_MS, pdTRUE, btn, btn_read_cb);
    xTimerStart(btn->read_timer, 0);

	return btn;
}

void button_add_short_press(button_t *btn, uint8_t press_cnt, button_cb cb, void *args)
{
    uint8_t i = 0;
    for ( i = 0; i < btn->short_press_cb_cnt; i++ )
    {
        if ( btn->short_press_cb[i].cb == cb && i == press_cnt - 1)
            return;
    }

    btn_cb_t *tmp = NULL;
    if ( btn->short_press_cb_cnt > 0 )
    {
        tmp = (btn_cb_t *) calloc( btn->short_press_cb_cnt,  sizeof(btn_cb_t));
        memcpy( tmp, btn->short_press_cb, (btn->short_press_cb_cnt) * sizeof(btn_cb_t));
    }
    
    btn->short_press_cb_cnt++;
    btn->short_press_cb = (btn_cb_t *) malloc(btn->short_press_cb_cnt * sizeof(btn_cb_t) );
    memset( btn->short_press_cb, 0, btn->short_press_cb_cnt * sizeof(btn_cb_t));
    
    if ( btn->short_press_cb_cnt > 1 )
    {
        memcpy( btn->short_press_cb, tmp, (btn->short_press_cb_cnt - 1) * sizeof(btn_cb_t));
    }
    free( tmp);

    btn->short_press_cb[ btn->short_press_cb_cnt - 1].cb = cb;
    btn->short_press_cb[ btn->short_press_cb_cnt - 1].args = args;
    btn->short_press_cb[ btn->short_press_cb_cnt - 1].idx = press_cnt;
}

void button_add_long_press(button_t *btn, uint16_t timeout, button_cb cb, void *args)
{
    uint8_t i = 0;
    for ( i = 0; i < btn->long_press_cb_cnt; i++ )
    {
        if ( btn->long_press_cb[i].cb == cb && btn->long_press_cb[i].timeout == timeout )
            return;
    }

    btn_cb_t *tmp = NULL;
    if ( btn->long_press_cb_cnt > 0 )
    {
        tmp = (btn_cb_t *) calloc( btn->long_press_cb_cnt, sizeof(btn_cb_t));
        memcpy( tmp, btn->long_press_cb, (btn->long_press_cb_cnt) * sizeof(btn_cb_t));
    }
    
    btn->long_press_cb_cnt++;
    btn->long_press_cb = (btn_cb_t *) malloc(btn->long_press_cb_cnt * sizeof(btn_cb_t) );
    memset( btn->long_press_cb, 0, btn->long_press_cb_cnt * sizeof(btn_cb_t));
    
    if ( btn->long_press_cb_cnt > 1 )
    {
        memcpy( btn->long_press_cb, tmp, (btn->long_press_cb_cnt - 1) * sizeof(btn_cb_t));
    }
    free( tmp);
	
    btn->long_press_cb[ btn->long_press_cb_cnt - 1].cb = cb;
    btn->long_press_cb[ btn->long_press_cb_cnt - 1].args = args;
    btn->long_press_cb[ btn->long_press_cb_cnt - 1].timeout = timeout;    

    // sort by timeout

    if ( btn->long_press_cb_cnt > 1) 
    {
        for ( i = 0; i < btn->long_press_cb_cnt; i++)
        {
            uint8_t j = 0;
            for ( j = i + 1; j < btn->long_press_cb_cnt; j++ )
            {
                if ( btn->long_press_cb[i].timeout > btn->long_press_cb[j].timeout )
                {
                    btn_cb_t *t = (btn_cb_t *) calloc( 1, sizeof(btn_cb_t));
                    memcpy(t, &btn->long_press_cb[i], sizeof(btn_cb_t));
                    memcpy(&btn->long_press_cb[i], &btn->long_press_cb[j], sizeof(btn_cb_t));
                    memcpy(&btn->long_press_cb[j], t, sizeof(btn_cb_t));
                    free(t);
                }
            } 
        }
    }    
}
