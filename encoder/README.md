# Component: encoder

энкодер ЕС11 за полный оборот генерирует 20 серий импульсов. А это значит, что каждый шаг эквивалентен повороту на 18°. Помимо этого, вал энкодера фиксируется в каждом положении между каждой серией импульсов.
http://codius.ru/articles/%D0%98%D0%BD%D0%BA%D1%80%D0%B5%D0%BC%D0%B5%D0%BD%D1%82%D0%B0%D0%BB%D1%8C%D0%BD%D1%8B%D0%B9_%D1%8D%D0%BD%D0%BA%D0%BE%D0%B4%D0%B5%D1%80_%D0%BF%D1%80%D0%B8%D0%BD%D1%86%D0%B8%D0%BF_%D0%B4%D0%B5%D0%B9%D1%81%D1%82%D0%B2%D0%B8%D1%8F_%D1%81%D1%85%D0%B5%D0%BC%D1%8B_%D0%BF%D0%BE%D0%B4%D0%BA%D0%BB%D1%8E%D1%87%D0%B5%D0%BD%D0%B8%D1%8F_%D1%80%D0%B0%D0%B1%D0%BE%D1%82%D0%B0_%D1%81_Arduino

1. define encoder handle
    encoder_handle_t enc_h;

2. Fill encoder init structure

    encoder_config_t enc_cfg = {
        .pin_btn = 13,
        .pin_clk = 4,	
        .pin_dt = 0,
        .left = enc_left_cb,
	    .cb_left_ctx = "left",
	    .right = enc_right_cb,
	    .cb_right_ctx = "right",
	    .press = enc_press_cb,
	    .cb_press_ctx = "press",
    };

3. Create and initialize encoder

    enc_h = encoder_init(enc_cfg);

callback functions

void enc_left_cb() {
    ESP_LOGI(TAG, __func__);

}

void enc_right_cb() {
    ESP_LOGI(TAG, __func__);

}

void enc_press_cb() {
    ESP_LOGI(TAG, __func__);

}
