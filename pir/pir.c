
#include "pir.h"


#define X_DELETE_TIMER(tmr) { if ( tmr ) {xTimerStop(tmr, 0); xTimerDelete( tmr, 0 );} }
#define X_RESET_TIMER(tmr, period) { xTimerStop(tmr, 0); \
							 xTimerChangePeriod(tmr, period, portMAX_DELAY); \
							 xTimerReset( tmr, 0 ); }

static const char *TAG = "PIR";

static void pir_update_timer_low_interval(pir_handle_t pir_h, uint16_t val);
static void pir_update_timer_hig_interval(pir_handle_t pir_h, uint16_t val);

static void task_cb(void *arg) {

	pir_t *pir = (pir_t *) arg;

	int delay = 10 / portTICK_PERIOD_MS;
	while (1) {
		if ( pir->taskq && pir->argq /*&& uxQueueMessagesWaiting(pir->taskq) != 0*/) {
			void (*task)(void *);
			void *arg;		
			if ( xQueueReceive(pir->taskq, &task, 0) == pdPASS &&
				 xQueueReceive(pir->argq, &arg, 0) == pdPASS ) 
				 {
					if ( task ) task(arg);
				 }
		}
		vTaskDelay( delay );
	}
	vTaskDelete(NULL);
}

// callback when pir send HIGH
static void pir_high_cb(void *arg)
{
	pir_t *pir = (pir_t *) arg;
	if ( pir->high_cb) 
		pir->high_cb( pir->cb_high_ctx );
}

// callback when pir send LOW
static void pir_low_cb(void *arg)
{
	pir_t *pir = (pir_t *) arg;
	if ( pir->low_cb )
		pir->low_cb( pir->cb_low_ctx );
}

static void pir_tmr_low_cb(xTimerHandle tmr)
{
	pir_t *pir = (pir_t *) pvTimerGetTimerID(tmr);
	if ( pir->tmr_low_cb ) {
		//ESP_LOGI(TAG, "pir->tmr_low_cb add %p", pir->tmr_low_cb);
		pir->tmr_low_cb( pir->cb_tmr_low_ctx );
	}
}

static void pir_tmr_high_cb(xTimerHandle tmr)
{
	pir_t *pir = (pir_t *) pvTimerGetTimerID(tmr);
	
	if ( pir->tmr_high_cb ) {
		//ESP_LOGI(TAG, "pir->tmr_high_cb add %p", pir->tmr_high_cb);
		pir->tmr_high_cb( pir->cb_tmr_high_ctx );
	}
}




static void gpio_poll(void *arg){
	pir_t *pir = (pir_t *) arg;

	while (1) {
		int level = gpio_get_level( pir->pin );
	
		if ( level == 1 ) {	
			if ( pir->active_level ==  PIR_LEVEL_HIGH || pir->active_level ==  PIR_LEVEL_ANY) {   // 
				pir_high_cb( pir );

				// stop tmr_low
				// if (pir->tmr_low) { 
				// 	xTimerStop(pir->tmr_low, 0);
				// 	xTimerDelete( pir->tmr_low, 0 );	
				// }
				X_DELETE_TIMER(pir->tmr_low); 
				
				if (pir->tmr_high && pir->tmr_high_cb) {  // restart timer if timer already created
					// xTimerStop(pir->tmr_high, 0);
					// xTimerReset(pir->tmr_high, 0);	
					X_RESET_TIMER( pir->tmr_high, pir->interval_high );	
				} else {
					// create and start timer
					if ( pir->tmr_high && pir->tmr_high_cb) {
						pir->tmr_high = xTimerCreate("pir_tmr_high", pir->interval_high, pdFALSE, pir, pir_tmr_high_cb);    // callback timer stop
						xTimerStart(pir->tmr_high, 0);
					}
				}			
			} else {
				pir_low_cb( pir );
			}
		} else {
			if ( pir->active_level ==  PIR_LEVEL_HIGH || pir->active_level ==  PIR_LEVEL_ANY ) {   // 
				pir_low_cb( pir );

				// stop tmr high
				// if (pir->tmr_high) { 
				// 	xTimerStop(pir->tmr_high, 0);
				// 	xTimerDelete( pir->tmr_high, 0 );
				// }	
				X_DELETE_TIMER(pir->tmr_high); 
				
				if (pir->tmr_low && pir->tmr_low_cb) {  // restart timer if timer already created
					// xTimerStop(pir->tmr_low, 0);
					// xTimerReset(pir->tmr_low, 0);
					X_RESET_TIMER( pir->tmr_low, pir->interval_low );		
				} else {
					// create and start timer
					if ( pir->tmr_low && pir->tmr_low_cb ) {
						pir->tmr_low = xTimerCreate("pir_tmr_low", pir->interval_low, pdFALSE, pir, pir_tmr_low_cb);    // callback timer stop
						xTimerStart(pir->tmr_low, 0);
					}
				}			
			} else {
				pir_high_cb( pir );
			}		

		}
		vTaskDelay( pir->poll_interval  );
	}
	vTaskDelete( NULL );
}

static void IRAM_ATTR pir_isr_handler(void *arg) {
	portBASE_TYPE HPTaskAwoken = pdFALSE;
	BaseType_t xHigherPriorityTaskWoken;

	pir_t *pir = (pir_t *) arg;
	int level = gpio_get_level( pir->pin );
	
	if (pir->taskq != NULL && pir->argq != NULL ) {
            void *func;
            void *arg;
			if ( level == 1 ) {
				if ( pir->active_level ==  PIR_LEVEL_HIGH || pir->active_level == PIR_LEVEL_ANY ) {   // 
					func = pir_high_cb;
					arg = pir;

					// stop tmr_low
					if (pir->tmr_low ) { 
						xTimerStopFromISR(pir->tmr_low, &HPTaskAwoken);
						xTimerDelete( pir->tmr_low, 0 );
					}
 					// restart timer  high if timer already created
					if (pir->tmr_high && pir->tmr_high_cb) { 
						xTimerStopFromISR(pir->tmr_high, &HPTaskAwoken);
						xTimerChangePeriodFromISR(pir->tmr_high, pir->interval_high, &HPTaskAwoken);
						xTimerResetFromISR(pir->tmr_high, &HPTaskAwoken);		
					} else {
						// create and start timer
						if ( pir->tmr_high && pir->tmr_high_cb ) {
							pir->tmr_high = xTimerCreate("pir_tmr_high", pir->interval_high, pdFALSE, pir, pir_tmr_high_cb);    // callback timer stop
							xTimerStartFromISR(pir->tmr_high, &HPTaskAwoken);
						}
					}			
				} else {
					func = pir_low_cb;
					arg = pir;					
				}		
			} else {
				// level 0
				if ( pir->active_level ==  PIR_LEVEL_LOW || pir->active_level == PIR_LEVEL_ANY ) {   // 
					func = pir_low_cb;
					arg = pir;		
					// stop tmr high
					if (pir->tmr_high ) { 
						xTimerStopFromISR(pir->tmr_high, &HPTaskAwoken);
						xTimerDelete( pir->tmr_high, 0 );
					}				

					if (pir->tmr_low && pir->tmr_low_cb ) {  // restart timer if timer already created
						xTimerStopFromISR(pir->tmr_low, &HPTaskAwoken);
						xTimerChangePeriodFromISR(pir->tmr_low, pir->interval_low, &HPTaskAwoken);
						xTimerResetFromISR(pir->tmr_low, &HPTaskAwoken);		
					} else {
						// create and start timer
						if ( pir->tmr_low && pir->tmr_low_cb  ) {
							pir->tmr_low = xTimerCreate("pir_tmr_low", pir->interval_low, pdFALSE, pir, pir_tmr_low_cb);    // callback timer stop
							xTimerStartFromISR(pir->tmr_low, &HPTaskAwoken);
						}
					}			
				} else {
					func = pir_high_cb;
					arg = pir;						
				}		
			}
            xQueueOverwriteFromISR(pir->taskq, &func, &xHigherPriorityTaskWoken);
			xQueueOverwriteFromISR(pir->argq, &arg, &xHigherPriorityTaskWoken);
    }

	/*
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portEND_SWITCHING_ISR(pdTRUE);
    }	*/
	portEND_SWITCHING_ISR( HPTaskAwoken == pdTRUE );
}

static void pir_enable(pir_handle_t _pir) {
	pir_t *pir = (pir_t *) _pir;
	pir->status = PIR_ENABLED;

	if ( pir->type == PIR_ISR  ) {
		if ( !pir->task ) 
			xTaskCreate( pir->task_cb, "pir_task", 1024, pir, 10, &pir->task);

		if ( !pir->taskq ) 
			pir->taskq = xQueueCreate(1, sizeof(void *));

		if ( !pir->argq ) 
			pir->argq =  xQueueCreate(1, sizeof(void *));
					
		if ( !pir->tmr_low && pir->tmr_low_cb )
			pir->tmr_low = xTimerCreate("pir_tmr_low", pir->interval_low, pdFALSE, pir, pir_tmr_low_cb);
					
		if ( !pir->tmr_high && pir->tmr_high_cb )
			pir->tmr_high = xTimerCreate("pir_tmr_high", pir->interval_high, pdFALSE, pir, pir_tmr_high_cb);

		gpio_install_isr_service(0);
    	gpio_isr_handler_add( pir->pin, pir_isr_handler, pir);   // обработчик прерывания, в качестве аргумента передается указатель на сам объект pir
	} else {
		// polling gpio task
		// _pir->task_cb
		if ( pir->type == PIR_POLL )
			xTaskCreate( gpio_poll, "pir_task", 1024, pir, 10, &pir->task_poll);
	} 
}

static void pir_disable(pir_handle_t _pir) {
	pir_t *pir = (pir_t *) _pir;

	pir->status = PIR_DISABLED;

	if ( pir->taskq ) {
		vQueueDelete( pir->taskq );
		pir->taskq = NULL;
	}

	if ( pir->argq ) {
		vQueueDelete( pir->argq );
		pir->argq = NULL;
	}

	if ( pir->task ) {
		vTaskDelete( pir->task );
		pir->task = NULL;
	}

	if ( pir->type == PIR_ISR  ) {
		if ( pir->tmr_low ) {
			xTimerStop( pir->tmr_low, 0 );
			xTimerDelete( pir->tmr_low, 0 );
		}		
		if ( pir->tmr_high ) {
			xTimerStop( pir->tmr_high, 0 );
			xTimerDelete( pir->tmr_high, 0 );
		}
    	gpio_isr_handler_remove( pir->pin );   // обработчик прерывания, в качестве аргумента передается указатель на сам объект pir
	} else {
		// polling gpio task
		// _pir->task_cb
		if ( pir->type == PIR_POLL && pir->task_poll ) {
			if ( pir->task_poll ) { 
				vTaskDelete( pir->task_poll );
				pir->task_poll = NULL;
			}	
		}	
	} 
}

pir_handle_t pir_init(pir_conf_t pir_conf){
	pir_t * _pir = calloc(1, sizeof(pir_t));
	
	_pir->pin = pir_conf.pin;
	_pir->active_level = pir_conf.active_level;
	_pir->type = pir_conf.type;
	_pir->status = PIR_DISABLED;
	_pir->cb_high_ctx = pir_conf.cb_high_ctx;
	_pir->high_cb = pir_conf.high_cb;	
	
	_pir->cb_low_ctx = pir_conf.cb_low_ctx;
	_pir->low_cb = pir_conf.low_cb;
	
	_pir->tmr_low = NULL;
	_pir->tmr_high = NULL;

	if ( _pir->type == PIR_ISR) {
		_pir->interval_low = pir_conf.interval_low * 1000 / portTICK_PERIOD_MS;
		_pir->interval_high = pir_conf.interval_high * 1000 / portTICK_PERIOD_MS;
	} else {	
		_pir->poll_interval = pir_conf.poll_interval / portTICK_PERIOD_MS;
	}

	_pir->cb_tmr_low_ctx = pir_conf.cb_tmr_low_ctx;
	_pir->cb_tmr_high_ctx = pir_conf.cb_tmr_high_ctx;
	_pir->tmr_low_cb = pir_conf.tmr_low_cb;
	_pir->tmr_high_cb = pir_conf.tmr_high_cb;
	
	_pir->argq = NULL;
 	_pir->taskq = NULL;
	_pir->task = NULL;
	_pir->task_poll = NULL;

	_pir->enable = pir_enable;
	_pir->disable = pir_disable;

	_pir->task_cb = task_cb;
	_pir->set_interval_low = pir_update_timer_low_interval;
	_pir->set_interval_high = pir_update_timer_hig_interval;

    // настройка  gpio
    
    gpio_config_t gpio_conf;
	if ( pir_conf.active_level == PIR_LEVEL_HIGH)
    	gpio_conf.intr_type = GPIO_INTR_POSEDGE;	// GPIO_INTR_HIGH_LEVEL
	else if ( pir_conf.active_level == PIR_LEVEL_LOW )	
		gpio_conf.intr_type = GPIO_INTR_NEGEDGE;  	// GPIO_INTR_LOW_LEVEL
	else if ( pir_conf.active_level == PIR_LEVEL_ANY )
		gpio_conf.intr_type = GPIO_INTR_ANYEDGE;  	
	else	
		gpio_conf.intr_type = GPIO_INTR_DISABLE;  		
		
	//gpio_conf.intr_type = GPIO_INTR_ANYEDGE;  	// GPIO_INTR_LOW_LEVEL

    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pin_bit_mask = (1ULL << _pir->pin);
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&gpio_conf);

	pir_enable( _pir );

	return (pir_handle_t ) _pir;
}


void pir_update_timer_low_interval(pir_handle_t pir_h, uint16_t val){
	if ( val <=0 ) return;
	pir_t *pir = (pir_t *)pir_h;
	pir->interval_low = val * 1000 / portTICK_PERIOD_MS;
}

void pir_update_timer_hig_interval(pir_handle_t pir_h, uint16_t val){
	if ( val <=0 ) return;
	pir_t *pir = (pir_t *)pir_h;
	pir->interval_high = val * 1000 / portTICK_PERIOD_MS;
}
