#include "ipwm.h"



static const char *TAG = "PWM";

static uint16_t period;

void pwm_begin(uint16_t freq_hz, uint8_t ch_cnt, const uint32_t *channels){
    ESP_LOGD(TAG, "%s started...", __func__);

    // !!! WORKAROUND OF NON WORKING PWM !!!
    REG_WRITE(PERIPHS_DPORT_BASEADDR, (REG_READ(PERIPHS_DPORT_BASEADDR) & ~0x1F) | 0x1);
    
	period = 1000000 / freq_hz;  // Hz to period, Just freq_hz is useful
    uint32_t *duties = malloc( sizeof(uint32_t) * ch_cnt);
    for (uint8_t i=0;i<ch_cnt;i++) duties[i] = period;

    pwm_init(period, duties, ch_cnt, channels);

    float *phases = malloc( sizeof(float) * ch_cnt);
    memset(phases, 0, sizeof(float) * ch_cnt);
    pwm_set_phases(phases);

    pwm_start();
    free(phases);
    free(duties);
}


void pwm_write(uint8_t ch, uint16_t duty) {
    uint16_t real_duty = duty*period/MAX_DUTY;
    pwm_set_duty(ch, real_duty);
}

uint16_t pwm_state(uint8_t ch) {
    uint32_t real_duty;
    pwm_get_duty(ch, &real_duty);
    return (real_duty * MAX_DUTY) / period;
}

/*
void pwm_task(){

    pwm_begin(PWM_FREQ_HZ, PIN_PWM_CNT, pwm_pins);

    uint32_t duty = 0;
    uint8_t direction = 0;
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    while (1) {
        pwm_write(0, duty);
        pwm_write(1, duty);
        pwm_write(2, duty);

        if ( direction == 0 ) {
            duty += STEP;
            if ( duty == MAX_DUTY ) {
                    direction = 1;
                    //vTaskDelay(2000 / portTICK_RATE_MS);
                    vTaskDelayUntil( &xLastWakeTime, ( 500 / portTICK_RATE_MS ) );
             }
             
        } else {
            duty -= STEP;
            if ( duty == 1 ) {
                    direction = 0;
                    //vTaskDelay(2000 / portTICK_RATE_MS);
                    vTaskDelayUntil( &xLastWakeTime, ( 500 / portTICK_RATE_MS ) );
            }
        }

        
        
        vTaskDelay(FADE_TIME / portTICK_RATE_MS);
    }
}
*/