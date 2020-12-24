#include "effects.h"

#ifdef CONFIG_RGB_EFFECTS

static const char *TAG = "COLOREFFECT";

#define RGB_EFFECTS_TASK_PRIORITY 10 // 6 слишком мало 
#define RGB_EFFECTS_TASK_STACK_SIZE 512

  // current effect
static effects_t* effects = NULL;
void* pcolors = NULL;

void effects_set_effect_by_name(const char  *name );
void effects_set_effect( int8_t id );
void _manage_color_effect(effect_t *e);

void effects_next_effect();
void effects_prev_effect();
void effects_stop_effect();



effects_t* effects_init(void *rgbctrl, effect_set_color_hsv_f *cb) {
    effects = calloc(1, sizeof(effects_t));
    effects->rgbctrl = rgbctrl;
    //effects->effect = calloc( COLOR_EFFECTS_MAX, sizeof(effect_t));
    effects->effect = calloc( 1, sizeof(effect_t));
    effects->set_color_hsv = cb;
    effects->task = NULL;
    effects->task_cb = NULL;
    effects->effect_id = COLOR_EFFECTS_MAX-1;
    
    // TODO: понять, зачем мы копируем, если уже есть массив определений color_effects
    //for ( int i = 0; i < COLOR_EFFECTS_MAX; i++) {
        effect_t *e = effects->effect;// + i;
        //effect_t *ce = &color_effects[i];
        //strcpy(e->name, ce->name);
        //memcpy(&e->type, &ce->type, sizeof( color_effect_e ));
        //memcpy(&e->fadeup_delay, &ce->fadeup_delay, sizeof(int16_t));
        //memcpy(&e->fadedown_delay, &ce->fadedown_delay, sizeof(int16_t));
        //e->cb = ce->cb;
        e->colors = NULL;
        e->colors_cnt = 0;
        e->hsv.h = 0; e->hsv.s = 100; e->hsv.v = 0;
        e->pe = effects;
    //}

    effects->set = effects_set_effect;
    effects->set_by_name = effects_set_effect_by_name;
    effects->next = effects_next_effect;
    effects->prev = effects_prev_effect;
    effects->stop = effects_stop_effect;
    

    return effects;
}

void effects_set_effect_by_name(const char  *name ) {
    for (int i=0;i<COLOR_EFFECTS_MAX;i++) {
        //effect_t *e = effects->effect + i;
        //if ( strcmp( e->name, name) == ESP_OK ) {
        if ( strcmp( color_effects[i].name, name) == ESP_OK ) {
            effects->set( i );
            break;
        }
    }
}

static void cleanup_effect_colors(effects_t *ee) {
	if ( pcolors != NULL ) {
		free(pcolors);
		pcolors = NULL;
	}

    if ( ee->task != NULL ) {
        vTaskDelete( ee->task );
        ee->task = NULL;
        ee->task_cb = NULL;
    }
}


static void effects_data_mqtt_publish(effect_t *e)
{
    
    rgbcontrol_queue_t *data = (rgbcontrol_queue_t *) calloc(1, sizeof(rgbcontrol_queue_t));

    data->type = RGB_EFFECT_ID;
    data->data = (char *) e->name;

    
    if ( rgbcontrol_color_mqtt_send_queue != NULL ) 
        xQueueSendToBack(rgbcontrol_color_mqtt_send_queue, data, 0);

    free(data);        

}

void effects_set_effect( int8_t id ){
    if ( id < 0 || id >= COLOR_EFFECTS_MAX ) return;
    cleanup_effect_colors(effects);
    
    effect_t *e = effects->effect;// + id;

    // fill effect from array effects
    strcpy(e->name, color_effects[id].name);
    e->type = color_effects[id].type;
    e->fadeup_delay = color_effects[id].fadeup_delay;   
    e->fadedown_delay = color_effects[id].fadedown_delay;   
    e->cb = color_effects[id].cb;   
        //memcpy(&e->fadeup_delay, &ce->fadeup_delay, sizeof(int16_t));
        //memcpy(&e->fadedown_delay, &ce->fadedown_delay, sizeof(int16_t));
        //e->cb = ce->cb;

    effects->task_cb = e->cb;

    ESP_LOGI(TAG, "%s: %s", __func__, color_effects[id].name);

    if ( effects->task_cb != NULL && id != EFFECT_STOP) {
        xTaskCreate( effects->task_cb, "effect_task", RGB_EFFECTS_TASK_STACK_SIZE, (effect_t *)e, RGB_EFFECTS_TASK_PRIORITY, &effects->task);
    }        

    
    if ( effects->effect_id != id ) 
    {
        // эта для публикации в mqtt новых значений эффекта
        effects_data_mqtt_publish(e);
    }

    effects->effect_id = id;

}

void effect_jump3(void *arg) 
{


    effect_t *e = (effect_t *)arg;
    e->colors_cnt = 3;
    e->colors = calloc( e->colors_cnt, sizeof( e->colors ));
	e->colors[0] = 0; e->colors[1] = 120; e->colors[2] = 240;
    e->type = JUMP;
    _manage_color_effect(e);
}

void effect_jump7(void *arg){
    effect_t *e = (effect_t *)arg;
    e->colors_cnt = 7;
    e->colors = calloc( e->colors_cnt, sizeof( e->colors ));
	memcpy( e->colors, hsv_colors_7, e->colors_cnt * sizeof( e->colors));
    e->type = JUMP;
    _manage_color_effect(e);
}

void effect_jump12(void *arg){
    effect_t *e = (effect_t *)arg;
    e->colors_cnt = 12;
    e->colors = calloc( e->colors_cnt, sizeof( e->colors ));
	
	for (uint8_t i = 0; i < e->colors_cnt; i++) {
		e->colors[i] = 30*i;     	// red	
	}
    
    e->type = JUMP;
    _manage_color_effect(e);
}

void effect_rndjump7(void *arg){
    effect_t *e = (effect_t *)arg;
    e->colors_cnt = 7;
    e->colors = calloc( e->colors_cnt, sizeof( e->colors ));
	memcpy( e->colors, hsv_colors_7, e->colors_cnt * sizeof( e->colors));
    e->type = RANDOM_JUMP;
    _manage_color_effect(e);
}

void effect_rndjump12(void *arg){
    effect_t *e = (effect_t *)arg;
    e->colors_cnt = 12;
    e->colors = calloc( e->colors_cnt, sizeof( e->colors ));
	
	for (uint8_t i = 0; i < e->colors_cnt; i++) {
		e->colors[i] = 30*i;     	// red	
	}
    e->type = RANDOM_JUMP;
    _manage_color_effect(e);
}

void effect_fade3(void *arg){
    effect_t *e = (effect_t *)arg;
    e->colors_cnt = 3;
    e->colors = calloc( e->colors_cnt, sizeof( e->colors ));
	e->colors[0] = 0; e->colors[1] = 120; e->colors[2] = 240;
    e->type = FADE;
    _manage_color_effect(e);
}

void effect_fade7(void *arg){
    effect_t *e = (effect_t *)arg;
    e->colors_cnt = 7;
    e->colors = calloc( e->colors_cnt, sizeof( e->colors ));
	memcpy( e->colors, hsv_colors_7, e->colors_cnt * sizeof( e->colors));
    e->type = FADE;
    _manage_color_effect(e);
}

void effect_fade12(void *arg){
    effect_t *e = (effect_t *)arg;
    e->colors_cnt = 12;
    e->colors = calloc( e->colors_cnt, sizeof( e->colors ));
	
	for (uint8_t i = 0; i < e->colors_cnt; i++) {
		e->colors[i] = 30*i;     	// red	
	}
    e->type = FADE;
    _manage_color_effect(e);
}

void effect_rndfade7(void *arg){
    effect_t *e = (effect_t *)arg;
    e->colors_cnt = 7;
    e->colors = calloc( e->colors_cnt, sizeof( e->colors ));
	memcpy( e->colors, hsv_colors_7, e->colors_cnt * sizeof( e->colors));
    e->type = RANDOM_FADE;
    _manage_color_effect(e);
}

void effect_rndfade12(void *arg){
    effect_t *e = (effect_t *)arg;
    e->colors_cnt = 12;
    e->colors = calloc( e->colors_cnt, sizeof( e->colors ));
	
	for (uint8_t i = 0; i < e->colors_cnt; i++) {
		e->colors[i] = 30*i;     	// red	
	}
    e->type = RANDOM_FADE;
    _manage_color_effect(e);
}

void effect_wheel(void *arg){
    effect_t *e = (effect_t *)arg;
    e->colors_cnt = 360;
    e->colors = calloc( e->colors_cnt, sizeof( e->colors ));
	for (uint16_t i = 0; i < e->colors_cnt; i++) {
		e->colors[i] = i;     	
	}
    e->type = JUMP;
    _manage_color_effect(e);
}

void effect_rnd(void *arg){
    effect_t *e = (effect_t *)arg;
    e->colors_cnt = 360;
    e->colors = calloc( e->colors_cnt, sizeof( e->colors ));
	for (uint16_t i = 0; i < e->colors_cnt; i++) {
		e->colors[i] = i;     	
	}
    e->type = RANDOM_CB;
    _manage_color_effect(e);
}

void effect_stop(void *arg){
    effect_t *e = (effect_t *)arg;
    e->type = STOP;
    //e->hsv.h = 0; e->hsv.s = 0; e->hsv.v = 0;
    //e->pe->set_color_hsv( e->hsv );
}

static void calc_color_duty_and_dir(uint8_t *val, effect_direction_e *dir) {
	if ( *val == VAL_MIN) *dir = E_UP;
	if ( *dir == E_UP) (*val)++;
	if ( *val == VAL_MAX) *dir = E_DOWN;
	if ( *dir == E_DOWN) (*val)--;
}

void _manage_color_effect(effect_t *e) {
	effect_direction_e dir = E_UP, prev_dir = E_UP;
    e->hsv.h = 0;
    e->hsv.v = VAL_MAX;
    e->hsv.s = SAT_MAX;
    pcolors = e->colors;

    uint16_t mm = e->mm;
    while( 1 ) {
        if (e->type == JUMP ) {
            e->hsv.h = e->colors[mm];  
        } else if ( e->type == FADE ) {
            prev_dir = dir;
            calc_color_duty_and_dir( &e->hsv.v, &dir);
            e->hsv.h = e->colors[ mm ];
        } else if (e->type == RANDOM_JUMP ) {
            e->hsv.h = e->colors[ esp_random() % e->colors_cnt ];
        } else if ( e->type == RANDOM_FADE ) {
            prev_dir = dir;
            calc_color_duty_and_dir( &e->hsv.v, &dir);
        } else if ( e->type == RANDOM_CB ) {
            e->hsv.h = e->colors[ esp_random() % e->colors_cnt ];
            e->hsv.v = esp_random() % VAL_MAX;
        }

        e->pe->set_color_hsv( e->hsv );

        if ( e->type == JUMP || e->type == RANDOM_JUMP ) {
            ++mm;
            if ( mm == e->colors_cnt ) mm = 0;
        } else if ( e->type == FADE || e->type == RANDOM_FADE ) {
            if ( e->hsv.v == VAL_MIN ) {
                dir = E_UP;
                ++mm;
                if ( mm == e->colors_cnt ) mm = 0;
                if ( e->type == RANDOM_FADE )
                    e->hsv.h = e->colors[ esp_random() % e->colors_cnt ];
            }
        }
        e->mm = mm;
        if ( dir == E_UP ) {
            vTaskDelay( e->fadeup_delay / portTICK_RATE_MS);
        } else {    
            vTaskDelay( e->fadedown_delay / portTICK_RATE_MS );
        }
    }
    free( e->colors);
    e->colors = NULL;
    e->colors_cnt = 0;
    vTaskDelete( NULL );
}

void effects_next_effect(){
    if ( effects->effect_id < COLOR_EFFECTS_MAX ) effects->effect_id++;
    //effect_id++;
    if ( effects->effect_id == COLOR_EFFECTS_MAX ) effects->effect_id = 0;
    effects->set( effects->effect_id );
}

void effects_prev_effect(){
    if ( effects->effect_id > -1 ) effects->effect_id--;
    if ( effects->effect_id == -1 ) effects->effect_id = COLOR_EFFECTS_MAX-1;
    effects->set( effects->effect_id );
}

void effects_stop_effect(){
    cleanup_effect_colors(effects);
    effects->effect_id = COLOR_EFFECTS_MAX-1;
}




#endif