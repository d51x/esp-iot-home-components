sht21

http://we.easyelectronics.ru/blog/part/734.html

I2C вполне стандартный, работающий на частоте до 400кГц. Адрес датчика фиксирован — 1000000 или, другими словами, 0x80.

Команд для управления SHT21 всего 7:

0xE3 — Измерить температуру. При этом на время измерения линия SCL будет прижата к земле.  HOLD
0xE5 — Измерить влажность. Линия SCL так-же прижимается к земле на время измерения.        HOLD
0xF3 — Измерить температуру. На этот раз датчик не прижимает линию SCL на время замера.    NO HOLD
0xF5 — Измерить влажность. Линия SCL не прижимается.                                       NO HOLD 
0xE6 — Записать данные в пользовательский регистр.
0xE7 — Прочитать данные из пользовательского регистра.
0xFE — Перезагрузить датчик.

SHT21 умеет работать в двух режимах:

1) Датчик прижимает линию SCL к земле на время измерения и подготовки данных.
Довольно удобная фича. Можно легко организовать считывание данных, хоть на прерываниях (если на ножке SCL есть функция внешнего прерывания).

2) Датчик не прижимает линию SCL при измерении.
Можно просто расчитать задержку и прочитать данные в нужный момент. Или пинговать SHT21 — если не отвечает на адрес, значит еще занят.

Не рекомендуется опрашивать датчик слишком часто из-за его нагрева и низкой скорости реакции на изменения влажности. Минимальный рекомендуемый  интервал между измерениями 10..18 секунд.s



esp_err_t err = sht21_init();
while (true) {
    
    if ( sht21 == ESP_OK )
    {
        ESP_LOGI(TAG, "SHT21 Temperature: %0.2fC", (float) sht21_get_temp());
        ESP_LOGI(TAG, "SHT21 Humidity: %0.2f%%", (float) sht21_get_hum());
    } else {
        ESP_LOGI(TAG, "Try to initialize SHT21");
        sht21 = sht21_init();
    }
    vTaskDelay(2000/ portTICK_RATE_MS);
}