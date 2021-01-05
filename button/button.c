#include "button.h"

static const char *TAG = "button";

#define IOT_CHECK(tag, a, ret)  if(!(a)) {                                             \
        ESP_LOGE(tag,"%s:%d (%s)", __FILE__, __LINE__, __FUNCTION__);      \
        return (ret);                                                                   \
        }
#define ERR_ASSERT(tag, param)  IOT_CHECK(tag, (param) == ESP_OK, ESP_FAIL)
#define POINT_ASSERT(tag, param, ret)    IOT_CHECK(tag, (param) != NULL, (ret))




#define BUTTON_GLITCH_FILTER_TIME_MS   50

    static uint8_t pressed_count = 0;

static void button_press_cb(xTimerHandle tmr)
{
    button_cb_t *btn_cb = (button_cb_t *) pvTimerGetTimerID(tmr);
    button_t *btn = btn_cb->pbtn;
    // low, then restart
    if (btn->active_level == gpio_get_level(btn->io_num)) {
        btn->state = BUTTON_STATE_PRESSED;
        if (btn->taskq != NULL && btn->argq != NULL && btn->taskq_on && !btn_cb->on_press) {
            void *tmp = btn_cb->cb;
            xQueueOverwrite(btn->taskq, &tmp);
            xQueueOverwrite(btn->argq, &btn_cb->arg);
        } else if (btn_cb->cb) {
            btn_cb->cb(btn_cb->arg);
        }
    }
}

// поиск в массиве колбеков короткого нажатия по индексу
static void btn_short_press_by_count(uint8_t id, button_t *btn) 
{
    if ( btn->tap_psh2_cb.cb == NULL ) return;
    ESP_LOGD(TAG, "%s %d", __func__, id);
    button_cb *cb = (button_cb *)btn->tap_psh2_cb.cb;
    if ( cb[id] ) {
        void **args = btn->tap_psh2_cb.arg;
        cb[id](args[id]);
    }
}

// callback таймера для обработки коротких нажатий
static void button_tap_psh2_cb(xTimerHandle tmr)
{
    button_cb_t *btn_cb = (button_cb_t *) pvTimerGetTimerID(tmr);
    button_t *btn = btn_cb->pbtn;

    if ( btn->tap_psh2_cb.on_press > 0  ) {
        uint8_t cb_id = pressed_count - 1;
        if ( cb_id < btn->tap_psh2_cb.on_press) {
            if ( btn->state == BUTTON_STATE_IDLE ) {
                // сработает, только после отпускания кнопки 
                // если было одно короткое нажатие, но дольше времени таймера, то это короткое нажатие здесь не сработает
                // его обработка в button_tap_rls_cb
                btn_short_press_by_count(cb_id, btn);
            }    
        }
    }
    pressed_count = 0;    
}

static void button_tap_psh_cb(xTimerHandle tmr)
{
    button_cb_t *btn_cb = (button_cb_t *) pvTimerGetTimerID(tmr);
    button_t *btn = btn_cb->pbtn;
    xTimerStop(btn->tap_rls_cb.tmr, portMAX_DELAY);
    int lv = gpio_get_level(btn->io_num);



    if (btn->active_level == lv) {
        // True implies key is pressed
        btn->state = BUTTON_STATE_PUSH;
        pressed_count++;

        if (btn->press_serial_cb.tmr) {
            xTimerChangePeriod(btn->press_serial_cb.tmr, btn->serial_thres_sec * 1000 / portTICK_PERIOD_MS, portMAX_DELAY);
            xTimerReset(btn->press_serial_cb.tmr, portMAX_DELAY);
        }
       
        if ( btn->tap_psh2_cb.tmr ) {
           if( xTimerIsTimerActive( btn->tap_psh2_cb.tmr ) == pdFALSE ) { // change timer period
                xTimerStart(btn->tap_psh2_cb.tmr, 0);
           } else {
                xTimerStop(btn->tap_psh2_cb.tmr, portMAX_DELAY);
                xTimerReset(btn->tap_psh2_cb.tmr, portMAX_DELAY);
           }

        }
  
        if (btn->tap_psh_cb.cb) {
            btn->tap_psh_cb.cb(btn->tap_psh_cb.arg);
        }
        
    } else {
        // 50ms, check if this is a real key up
        if (btn->tap_rls_cb.tmr) {
            xTimerStop(btn->tap_rls_cb.tmr, portMAX_DELAY);
            xTimerReset(btn->tap_rls_cb.tmr, portMAX_DELAY);
        }
    }
}



static void button_tap_rls_cb(xTimerHandle tmr)
{
    
    button_cb_t *btn_cb = (button_cb_t *) pvTimerGetTimerID(tmr);
    button_t *btn = btn_cb->pbtn;
    xTimerStop(btn->tap_rls_cb.tmr, portMAX_DELAY);
    if (btn->active_level == gpio_get_level(btn->io_num)) {

    } else {
        // high, then key is up
        button_cb_t *pcb = btn->cb_head;
        while (pcb != NULL) {
            if (pcb->tmr != NULL) {
                xTimerStop(pcb->tmr, portMAX_DELAY);
            }
            pcb = pcb->next_cb;
        }
        if (btn->taskq != NULL && btn->argq != NULL && btn->taskq_on && uxQueueMessagesWaiting(btn->taskq) != 0 && btn->state != BUTTON_STATE_IDLE) {
            void (*task)(void *);
            void *arg;
            xQueueReceive(btn->taskq, &task, 0);
            xQueueReceive(btn->argq, &arg, 0);
            task(arg);
        } else {
            // добавил условие, если нажатие было коротким, но дольше, чем таймер отслеживания коротких нажатий
            // и не сработал другой callback, например, на удержание 1-2 сек, то запустим callback первого короткого нажатия
            if ( btn->state == BUTTON_STATE_PUSH && xTimerIsTimerActive(btn->tap_psh2_cb.tmr) == pdFALSE ) 
                btn_short_press_by_count(0, btn);
        }
        if (btn->press_serial_cb.tmr && btn->press_serial_cb.tmr != NULL) {
            xTimerStop(btn->press_serial_cb.tmr, portMAX_DELAY);
        }
        if (btn->tap_short_cb.cb && btn->state == BUTTON_STATE_PUSH) {
            btn->tap_short_cb.cb(btn->tap_short_cb.arg);
        }
        if (btn->tap_rls_cb.cb && btn->state != BUTTON_STATE_IDLE) {
            btn->tap_rls_cb.cb(btn->tap_rls_cb.arg);
        }
        btn->state = BUTTON_STATE_IDLE;
    }
}


static void button_press_serial_cb(xTimerHandle tmr)
{
    button_t *btn = (button_t *) pvTimerGetTimerID(tmr);
    if (btn->press_serial_cb.cb) {
        btn->press_serial_cb.cb(btn->press_serial_cb.arg);
    }
    xTimerChangePeriod(btn->press_serial_cb.tmr, btn->press_serial_cb.interval, portMAX_DELAY);
    xTimerReset(btn->press_serial_cb.tmr, portMAX_DELAY);
}


static void button_gpio_isr_handler(void *arg) {
    button_t *btn = (button_t *) arg;
    portBASE_TYPE HPTaskAwoken = pdFALSE;

    int level = gpio_get_level(btn->io_num);
    if (level == btn->active_level) {
        if (btn->tap_psh_cb.tmr) {
            xTimerStopFromISR(btn->tap_psh_cb.tmr, &HPTaskAwoken);
            xTimerResetFromISR(btn->tap_psh_cb.tmr, &HPTaskAwoken);
        }
        button_cb_t *pcb = btn->cb_head;
        while (pcb != NULL) {
            if (pcb->tmr != NULL) {
                xTimerStopFromISR(pcb->tmr, &HPTaskAwoken);
                xTimerResetFromISR(pcb->tmr, &HPTaskAwoken);
            }
            pcb = pcb->next_cb;
        }        
    } else {
        // 50ms, check if this is a real key up
        if (btn->tap_rls_cb.tmr) {
            xTimerStopFromISR(btn->tap_rls_cb.tmr, &HPTaskAwoken);
            xTimerResetFromISR(btn->tap_rls_cb.tmr, &HPTaskAwoken);
        }
    }

    portEND_SWITCHING_ISR( HPTaskAwoken == pdTRUE );
  
}


static void button_free_tmr(xTimerHandle *tmr)
{
    if (tmr && *tmr) {
        xTimerStop(*tmr, portMAX_DELAY);
        xTimerDelete(*tmr, portMAX_DELAY);
        *tmr = NULL;
    }
}

esp_err_t button_delete(button_handle_t btn_handle)
{
    POINT_ASSERT(TAG, btn_handle, ESP_ERR_INVALID_ARG);

    button_t *btn = (button_t *) btn_handle;
    gpio_set_intr_type(btn->io_num, GPIO_INTR_DISABLE);
    gpio_isr_handler_remove(btn->io_num);

    button_free_tmr(&btn->tap_rls_cb.tmr);
    button_free_tmr(&btn->tap_psh_cb.tmr);
    button_free_tmr(&btn->tap_short_cb.tmr);
    button_free_tmr(&btn->press_serial_cb.tmr);

    button_cb_t *pcb = btn->cb_head;
    while (pcb != NULL) {
        button_cb_t *cb_next = pcb->next_cb;
        button_free_tmr(&pcb->tmr);
        free(pcb);
        pcb = cb_next;
    }
    free(btn);
    return ESP_OK;
}


button_handle_t button_create(gpio_num_t gpio_num, button_active_t active_level) {
    //if ( gpio_num >= GPIO_MAX_NUM) return NULL;
    IOT_CHECK(TAG, gpio_num < GPIO_NUM_MAX, NULL);
    ESP_LOGD(TAG, __func__);
    button_t *btn = (button_t *) calloc(1, sizeof(button_t));  // выделили память под button структуру
    POINT_ASSERT(TAG, btn, NULL);
    btn->active_level = active_level;
    btn->io_num = gpio_num;
    btn->state = BUTTON_STATE_IDLE;
    btn->taskq_on = 0;
    btn->taskq = xQueueCreate(1, sizeof(void *));
    btn->argq = xQueueCreate(1, sizeof(void *));
    btn->tap_rls_cb.arg = NULL;
    btn->tap_rls_cb.cb = NULL;
    btn->tap_rls_cb.interval = BUTTON_GLITCH_FILTER_TIME_MS / portTICK_PERIOD_MS;
    btn->tap_rls_cb.pbtn = btn;
    btn->tap_rls_cb.tmr = xTimerCreate("btn_rls_tmr", btn->tap_rls_cb.interval, pdFALSE, &btn->tap_rls_cb, button_tap_rls_cb);
    
    btn->tap_psh_cb.arg = NULL;
    btn->tap_psh_cb.cb = NULL;
    btn->tap_psh_cb.interval = BUTTON_GLITCH_FILTER_TIME_MS / portTICK_PERIOD_MS;
    btn->tap_psh_cb.pbtn = btn;
    btn->tap_psh_cb.tmr = xTimerCreate("btn_psh_tmr", btn->tap_psh_cb.interval, pdFALSE, &btn->tap_psh_cb, button_tap_psh_cb);    
    
    // callback для отслеживания коротких нажатий, срабатывает при отпускании кнопки после короткого нажатия 
    // или после последнего из серии нескольких коротких нажатий
    btn->tap_psh2_cb.arg = NULL;        // TODO array
    btn->tap_psh2_cb.on_press = 0;      // TODO array
    btn->tap_psh2_cb.cb = NULL;         // TODO array
    btn->tap_psh2_cb.interval = 300 / portTICK_PERIOD_MS;  // array ?
    btn->tap_psh2_cb.pbtn = btn;
    btn->tap_psh2_cb.tmr = xTimerCreate("btn_psh2_tmr", btn->tap_psh2_cb.interval, pdFALSE, &btn->tap_psh2_cb, button_tap_psh2_cb);    
        
    // настройка  gpio
    gpio_install_isr_service(0);
    gpio_config_t gpio_conf;
    gpio_conf.intr_type = GPIO_INTR_ANYEDGE;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pin_bit_mask = (1ULL << gpio_num);
    if(btn->active_level){
        gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    }else{
        gpio_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
        gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    }
    gpio_config(&gpio_conf);
    gpio_isr_handler_add(gpio_num, button_gpio_isr_handler, btn);   // обработчик прерывания, в качестве аргумента передается указатель на сам объект button

    return (button_handle_t) btn;
}

esp_err_t button_rm_cb(button_handle_t btn_handle, button_cb_type_t type)
{
    button_t *btn = (button_t *) btn_handle;
    button_cb_t *btn_cb = NULL;
    if (type == BUTTON_CB_PUSH) {
        btn_cb = &btn->tap_psh_cb;
    } else if (type == BUTTON_CB_RELEASE) {
        btn_cb = &btn->tap_rls_cb;
    } else if (type == BUTTON_CB_TAP) {
        btn_cb = &btn->tap_short_cb;
    } else if (type == BUTTON_CB_SERIAL) {
        btn_cb = &btn->press_serial_cb;
    }
    btn_cb->cb = NULL;
    btn_cb->arg = NULL;
    btn_cb->pbtn = btn;
    button_free_tmr(&btn_cb->tmr);
    return ESP_OK;
}

esp_err_t button_set_serial_cb(button_handle_t btn_handle, uint32_t start_after_sec, TickType_t interval_tick, button_cb cb, void *arg)
{
    
    button_t *btn = (button_t *) btn_handle;
    btn->serial_thres_sec = start_after_sec;
    if (btn->press_serial_cb.tmr == NULL) {
        btn->press_serial_cb.tmr = xTimerCreate("btn_serial_tmr", btn->serial_thres_sec * 1000 / portTICK_PERIOD_MS, pdFALSE, btn, button_press_serial_cb);
    }
    btn->press_serial_cb.arg = arg;
    btn->press_serial_cb.cb = cb;
    btn->press_serial_cb.interval = interval_tick;
    btn->press_serial_cb.pbtn = btn;
    xTimerChangePeriod(btn->press_serial_cb.tmr, btn->serial_thres_sec * 1000 / portTICK_PERIOD_MS, portMAX_DELAY);
    return ESP_OK;
}

esp_err_t button_set_evt_cb(button_handle_t btn_handle, button_cb_type_t type, button_cb cb, void *arg)
{
    POINT_ASSERT(TAG, btn_handle, ESP_ERR_INVALID_ARG);
    button_t *btn = (button_t *) btn_handle;
    if (type == BUTTON_CB_PUSH) {
        btn->tap_psh_cb.arg = arg;
        btn->tap_psh_cb.cb = cb;
        btn->tap_psh_cb.interval = BUTTON_GLITCH_FILTER_TIME_MS / portTICK_RATE_MS;
        btn->tap_psh_cb.pbtn = btn;
        xTimerChangePeriod(btn->tap_psh_cb.tmr, btn->tap_psh_cb.interval, portMAX_DELAY);
    } else if (type == BUTTON_CB_RELEASE) {
        btn->tap_rls_cb.arg = arg;
        btn->tap_rls_cb.cb = cb;
        btn->tap_rls_cb.interval = BUTTON_GLITCH_FILTER_TIME_MS / portTICK_RATE_MS;
        btn->tap_rls_cb.pbtn = btn;
        xTimerChangePeriod(btn->tap_rls_cb.tmr, btn->tap_psh_cb.interval, portMAX_DELAY);
    } else if (type == BUTTON_CB_TAP) {
        btn->tap_short_cb.arg = arg;
        btn->tap_short_cb.cb = cb;
        btn->tap_short_cb.interval = BUTTON_GLITCH_FILTER_TIME_MS / portTICK_RATE_MS;
        btn->tap_short_cb.pbtn = btn;
    } else if (type == BUTTON_CB_SERIAL) {
        button_set_serial_cb(btn_handle, 1, 1000 / portTICK_RATE_MS, cb, arg);
    }
    return ESP_OK;
}


esp_err_t button_add_on_press_cb(button_handle_t btn_handle, uint32_t press_sec, button_cb cb, void *arg)
{
    POINT_ASSERT(TAG, btn_handle, ESP_ERR_INVALID_ARG);
    IOT_CHECK(TAG, press_sec != 0, ESP_ERR_INVALID_ARG);
    button_t *btn = (button_t *) btn_handle;
    button_cb_t *cb_new = (button_cb_t *) calloc(1, sizeof(button_cb_t));
    POINT_ASSERT(TAG, cb_new, ESP_FAIL);
    cb_new->on_press = 1;
    cb_new->arg = arg;
    cb_new->cb = cb;
    cb_new->interval = press_sec * 1000 / portTICK_PERIOD_MS;
    cb_new->pbtn = btn;
    cb_new->tmr = xTimerCreate("btn_press_tmr", cb_new->interval, pdFALSE, cb_new, button_press_cb);
    cb_new->next_cb = btn->cb_head;
    btn->cb_head = cb_new;
    return ESP_OK;
}


// регистрация массива колбеков коротких нажатий
// установить кол-во и функции кол-беков при одинарном, двойном, и т.д. нажатии в указанный интервал
esp_err_t button_set_on_presscount_cb(button_handle_t btn_handle, uint32_t pressed_interval, uint8_t cbs_count, button_cb *cbs, void *args)
{
    POINT_ASSERT(TAG, btn_handle, ESP_ERR_INVALID_ARG);
    IOT_CHECK(TAG, cbs_count != 0, ESP_ERR_INVALID_ARG);
    IOT_CHECK(TAG, pressed_interval != 0, ESP_ERR_INVALID_ARG);
    button_t *btn = (button_t *) btn_handle;
    btn->tap_psh2_cb.arg = args; 
    btn->tap_psh2_cb.cb = cbs; 
    btn->tap_psh2_cb.on_press = cbs_count;
    btn->tap_psh2_cb.interval = pressed_interval / portTICK_PERIOD_MS; 
    btn->tap_psh2_cb.pbtn = btn;
    return ESP_OK;
}

esp_err_t button_add_on_release_cb(button_handle_t btn_handle, uint32_t press_sec, button_cb cb, void *arg)
{
    POINT_ASSERT(TAG, btn_handle, ESP_ERR_INVALID_ARG);
    IOT_CHECK(TAG, press_sec != 0, ESP_ERR_INVALID_ARG);
    button_t *btn = (button_t *) btn_handle;
    button_cb_t *cb_new = (button_cb_t *) calloc(1, sizeof(button_cb_t));
    POINT_ASSERT(TAG, cb_new, ESP_FAIL);
    btn->taskq_on = 1;
    cb_new->arg = arg;
    cb_new->cb = cb;
    cb_new->interval = press_sec * 1000 / portTICK_PERIOD_MS;
    cb_new->pbtn = btn;
    cb_new->tmr = xTimerCreate("btn_press_tmr", cb_new->interval, pdFALSE, cb_new, button_press_cb);
    cb_new->next_cb = btn->cb_head;
    btn->cb_head = cb_new;
    return ESP_OK;
}


button_handle_t configure_push_button(int gpio_num, button_active_t level)
{
    button_handle_t btn_handle = button_create(gpio_num, level);

    return btn_handle;
}
