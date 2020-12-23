
#include "encoder.h"



static const char *TAG = "ENCODER";

static void task_rotate_cb(void *arg) {
	encoder_t * enc = (encoder_t *)arg;
	int delay = 10 / portTICK_PERIOD_MS;
	while (1) {
		if ( enc->taskq != NULL && enc->argq != NULL /*&& uxQueueMessagesWaiting(enc->taskq) != 0*/) {
			void (*task)(void *);
			void *arg;		
			if ( xQueueReceive( enc->taskq, &task, 0) == pdPASS &&
				 xQueueReceive( enc->argq, &arg, 0) == pdPASS ) 
				 {
					if ( task != NULL )
						task(arg);
				 }
		}
		vTaskDelay( delay );
	}
	vTaskDelete(NULL);
}

static void IRAM_ATTR encoder_btn_isr_handler(encoder_handle_t enc_h) {
	encoder_t * enc = (encoder_t *)enc_h;
	
	portBASE_TYPE HPTaskAwoken = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken;
	
	void *func;
    void *arg;	
	
	// дребезг контактов !!!

	int level = gpio_get_level(  enc->pin_btn );
	if ( level == 1 ) {
		enc->state = ENCODER_STATE_PRESSED;
		func = enc->press;
		arg = enc;
		
		if ( enc->taskq != NULL && enc->argq != NULL ) {
		
			xQueueOverwriteFromISR( enc->taskq, &func, &xHigherPriorityTaskWoken);
			xQueueOverwriteFromISR( enc->argq, &arg, &xHigherPriorityTaskWoken);		
		}	
	
	} else {
		enc->state = ENCODER_STATE_IDLE;
	}
	
	portEND_SWITCHING_ISR( HPTaskAwoken == pdTRUE );
}

void IRAM_ATTR setCount(encoder_handle_t enc_h) {          // Устанавливаем значение счетчика
  encoder_t * enc = (encoder_t *)enc_h;
  if ( enc->rotate_state == 4 || enc->rotate_state == -4) {  // Если переменная state приняла заданное значение приращения
    enc->count += (int)(enc->rotate_state / 4);      // Увеличиваем/уменьшаем счетчик
    enc->lastTurn = esp_timer_get_time();            // Запоминаем последнее изменение
  }
}

static void IRAM_ATTR encoder_rotate_isr_handler(encoder_handle_t enc_h) {
	encoder_t * enc = (encoder_t *)enc_h;
	
	
	portBASE_TYPE HPTaskAwoken = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken;

	void *func = NULL;
	void *arg = NULL;	

	int level_clk = gpio_get_level(  enc->pin_clk );
	int level_dt = gpio_get_level(  enc->pin_dt );
	


	if ( level_clk && !enc->rotate_state)
	{
		if ( millis() - enc->lastTurn < ENCODER_ROTATE_DEBOUNCE_TIME) return;
		if ( level_dt) {		 
			//right
				enc->position++;
				enc->direction = ENCODER_ROTATE_RIGHT; 
				func = enc->right;
				arg = enc;
		} else {
				enc->position--;
				enc->direction = ENCODER_ROTATE_LEFT; 
				func = enc->left;
				arg = enc;
		}		
		if ( enc->position == 0 ) enc->direction = ENCODER_ROTATE_ZERO;
		if ( enc->taskq != NULL && enc->argq != NULL ) {
			xQueueOverwriteFromISR( enc->taskq, &func, &xHigherPriorityTaskWoken);
			xQueueOverwriteFromISR( enc->argq, &arg, &xHigherPriorityTaskWoken);		
		}
	}

	enc->rotate_state = level_clk;
	portEND_SWITCHING_ISR( HPTaskAwoken == pdTRUE );
}
/*
static void IRAM_ATTR encoder_dt_isr_handler(encoder_handle_t enc_h) {
	encoder_t * enc = (encoder_t *)enc_h;
	
	
	if (micros() - enc->lastTurn < enc->pause) return;

	portBASE_TYPE HPTaskAwoken = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken;

	int level_clk = gpio_get_level(  enc->pin_clk );
	int level_dt = gpio_get_level(  enc->pin_dt );
	
	if ( level_clk == 1 && level_dt == 0) {
		enc->rotate_state = -1;
		enc->lastTurn = micros();
	}
	else if ( level_clk == 0 && level_dt == 0 ) {
		enc->rotate_state = -2;
		enc->lastTurn = micros();
	}
	else if ( level_clk == 0 && level_dt == 1 ) {
		enc->rotate_state = -3;	
		enc->lastTurn = micros();
	}
	else if ( level_clk == 1 && level_dt == 1 ) {
		enc->rotate_state = -4;	
		enc->lastTurn = micros();
	}	

	if ( enc->rotate_state == -4)
	{
		void *func;
		void *arg;	

			enc->position--;
			enc->direction = ENCODER_ROTATE_LEFT; 
			func = enc->left;
			arg = enc;


		if ( enc->position == 0 ) enc->direction = ENCODER_ROTATE_ZERO;
		//send to queue encoder direction
		
		if ( enc->taskq != NULL && enc->argq != NULL ) {
		
			xQueueOverwriteFromISR( enc->taskq, &func, &xHigherPriorityTaskWoken);
			xQueueOverwriteFromISR( enc->argq, &arg, &xHigherPriorityTaskWoken);		
		}
	}
	portEND_SWITCHING_ISR( HPTaskAwoken == pdTRUE );
}
*/
static void reset_params(encoder_handle_t enc_h){
	encoder_t * enc = (encoder_t *)enc_h;
	enc->state = ENCODER_STATE_IDLE;
	enc->direction = ENCODER_ROTATE_ZERO;
	enc->position = 0;

	enc->pause    = ENCODER_ROTATE_DEBOUNCE_TIME;  
	enc->lastTurn = 0;   
	enc->rotate_state = 0;
	enc->count = 0;

}

static void encoder_enable( encoder_handle_t enc_h ) {
	encoder_t * enc = (encoder_t *)enc_h;
	enc->status = ENCODER_ENABLED;
	gpio_install_isr_service(0);
	gpio_isr_handler_add( enc->pin_clk, encoder_rotate_isr_handler, enc);
	//gpio_isr_handler_add( enc->pin_dt, encoder_rotate_isr_handler, enc);
	gpio_isr_handler_add( enc->pin_btn, encoder_btn_isr_handler, enc);
	reset_params( enc );	

	if (enc->taskq == NULL ) enc->taskq = xQueueCreate(1, sizeof(void *));
	if (enc->argq == NULL) 	enc->argq =  xQueueCreate(1, sizeof(void *));	
	if (enc->task_rotate == NULL ) xTaskCreate( enc->task_rotate_cb, "enc_rotate", 1024, enc, 10, &enc->task_rotate);
	
}

static void encoder_disable( encoder_handle_t enc_h ) {
	encoder_t * enc = (encoder_t *)enc_h;
	enc->status = ENCODER_DISABLED;
	gpio_isr_handler_remove( enc->pin_clk );
	//gpio_isr_handler_remove( enc->pin_dt );
	gpio_isr_handler_remove( enc->pin_btn );
	reset_params( enc );
	
	if ( enc->taskq != NULL ) {	vQueueDelete( enc->taskq ); enc->taskq = NULL;	}
	if ( enc->argq != NULL ) {	vQueueDelete( enc->argq );	enc->argq = NULL;	}
	if ( enc->task_rotate != NULL ) { vTaskDelete( enc->task_rotate );	enc->task_rotate = NULL; }

}

encoder_handle_t encoder_init(encoder_config_t enc_cfg){
	encoder_t * enc = calloc(1, sizeof(encoder_t));
	enc->status = ENCODER_DISABLED;
	reset_params( enc );
	
	enc->pin_btn = enc_cfg.pin_btn;
    enc->pin_clk = enc_cfg.pin_clk;	
    enc->pin_dt = enc_cfg.pin_dt;

	enc->left = enc_cfg.left;
	enc->cb_left_ctx = enc_cfg.cb_left_ctx;
	
	enc->right = enc_cfg.right;
	enc->cb_right_ctx = enc_cfg.cb_right_ctx;
	
	enc->press = enc_cfg.press;
	enc->cb_press_ctx = enc_cfg.cb_press_ctx;
	
	enc->enable = encoder_enable;
	enc->disable = encoder_disable;
	
	enc->taskq = NULL;
	enc->argq = NULL;
	enc->task_rotate = NULL;

	enc->task_rotate_cb = task_rotate_cb;

    gpio_config_t gpio_conf;
  	gpio_conf.intr_type = GPIO_INTR_POSEDGE;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pin_bit_mask = (1ULL << enc->pin_btn) | (1ULL << enc->pin_clk) | (1ULL <<  enc->pin_dt);
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&gpio_conf);

	// кнопку переиспользовать через button
	
	encoder_enable( enc );

	return (encoder_handle_t ) enc;
}