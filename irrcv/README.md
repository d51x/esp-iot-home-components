# Component: ir_receiver

irrcv_handle_t ir_rx;
	
	int pin = 5;
	int delay = 100;
	int btn_cnt = 2;
	
	init IR Receiver
        ir_rx = irrcv_init(pin, delay, btn_cnt);
	
	Add button callbacks
    if ( ir_rx != NULL ) {
				                btn_id	  btn_code   user_ctx      callback fun
        irrcv_add_button(ir_rx, 	 0, 0x00FF14EB, "button1", ir_button1_press);
        irrcv_add_button(ir_rx, 	 1, 0x00FF04FB, "button2", ir_button2_press);   
		
		Start IR Receiver to receive a code
        irrcv_start( ir_rx );
    } else {
        ESP_LOGE(TAG, "failed to init ir receiver");
    }
	
void ir_button1_press(void *arg) {
    ESP_LOGI(TAG, "%s %s", __func__, (char *)arg);
}

void ir_button2_press(void *arg) {
    ESP_LOGI(TAG, "%s %s", __func__, (char *)arg);
}	