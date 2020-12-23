

#include "mcp23017.h"
//
#ifdef CONFIG_COMPONENT_MCP23017


#define MCP23017_GPIO0   1 << 0     //0x0001
#define MCP23017_GPIO1   1 << 1     //0x0002
#define MCP23017_GPIO2   1 << 2     //0x0004
#define MCP23017_GPIO3   1 << 3     //0x0008
#define MCP23017_GPIO4   1 << 4     //0x0010
#define MCP23017_GPIO5   1 << 5     //0x0020
#define MCP23017_GPIO6   1 << 6     //0x0040
#define MCP23017_GPIO7   1 << 7     //0x0080
#define MCP23017_GPIO8   1 << 8     //0x0100
#define MCP23017_GPIO9   1 << 9     //0x0200
#define MCP23017_GPIO10  1 << 10    //0x0400
#define MCP23017_GPIO11  1 << 11    //0x0800
#define MCP23017_GPIO12  1 << 12    //0x1000
#define MCP23017_GPIO13  1 << 13    //0x2000
#define MCP23017_GPIO14  1 << 14    //0x4000
#define MCP23017_GPIO15  1 << 15    //0x8000

#define MCP23017_REG_IODIRA      0x00    // регистр, указыващий направления портов output/input
#define MCP23017_REG_IODIRB      0x01
#define MCP23017_REG_IPOLA       0x02    // инверсия ног, только для входов, 0 - по дефолту на порту 1, порт через 10k на +5v и кнопка на gnd дает на входе 0
                                         //                                  0 - по дефолту на порту 0, порт через 10k на gnd и кнопка на +5v дает на входе 1     
#define MCP23017_REG_IPOLB       0x03    
#define MCP23017_REG_GPINTENA    0x04    // прерывания на ногах
#define MCP23017_REG_GPINTENB    0x05
#define MCP23017_REG_DEFVALA     0x06    // дефолтные значения ног, прерывание сработает, если на ноге сигнал отличается от дефолтного
#define MCP23017_REG_DEFVALB     0x07
#define MCP23017_REG_INTCONA     0x08    // условия сработки прерывания на ногах
#define MCP23017_REG_INTCONB     0x09
#define MCP23017_REG_IOCONA      0x0A    // конфигурационный регистр
#define MCP23017_REG_IOCONB      0x0B
#define MCP23017_REG_GPPUA       0x0C    // подтяжка ног 100к
#define MCP23017_REG_GPPUB       0x0D
#define MCP23017_REG_INTFA       0x0E    // регистр флагов прерываний, покажет на какой ноге было прерывание
#define MCP23017_REG_INTFB       0x0F
#define MCP23017_REG_INTCAPA     0x10    // покажет что было на ноге в момент прерывания на этой ноге
#define MCP23017_REG_INTCAPB     0x11
#define MCP23017_REG_GPIOA       0x12    // состояние ног, когда было прерывание на ноге может уже быть другое значение и надо читать INTCAP, если работаем с прерываниями
#define MCP23017_REG_GPIOB       0x13
#define MCP23017_REG_OLATA       0x14    
#define MCP23017_REG_OLATB       0x15



static const char* TAG = "MCP23017";

#ifdef CONFIG_MCP23017_ISR

#define MCP23017_ISR_DELAY_MS 60

static void IRAM_ATTR mcp23027_isr_handler(void *arg) {
    portBASE_TYPE HPTaskAwoken = pdFALSE;
    BaseType_t xHigherPriorityTaskWoken;

    mcp23017_handle_t dev_h = (mcp23017_handle_t)arg;
    mcp23017_t *dev = (mcp23017_t *) dev_h;

    uint16_t data[2];
    mcp23017_read(dev_h, MCP23017_REG_INTFA, 4, &data );

    static uint32_t t = 0;

    if ( millis() - t >= MCP23017_ISR_DELAY_MS )
    {
        if (dev->taskq != NULL ) {
                xQueueOverwriteFromISR(dev->taskq, &data, &xHigherPriorityTaskWoken);
        }
        t = millis();
    }
    portEND_SWITCHING_ISR( HPTaskAwoken == pdTRUE );
}
#endif

#ifdef CONFIG_MCP23017_ISR
static void mcp23017_isr_cb(void *arg) {

	mcp23017_handle_t *dev_h = (mcp23017_handle_t *) arg;
    mcp23017_t * dev = (mcp23017_t *)dev_h;

	while (1) {
		if ( dev->taskq ) {
            uint16_t data[2];
			if ( xQueueReceive(dev->taskq, &data, 0) == pdPASS) 
				{
                    //ESP_LOGI(TAG, " interrput: %4d \t 0x%04X \t " BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN, data[0], data[0], BYTE_TO_BINARY(data[0] >> 8), BYTE_TO_BINARY(data[0]));
                    //ESP_LOGI(TAG, "gpio state: %4d \t 0x%04X \t " BYTE_TO_BINARY_PATTERN BYTE_TO_BINARY_PATTERN, data[1], data[1], BYTE_TO_BINARY(data[1] >> 8), BYTE_TO_BINARY(data[1]));

                    // check pins with interrupts
                    for ( uint8_t i = 0; i < 16; i++)
                    {
                        if ( BIT_CHECK( data[0], i) != 0)
                        {
                            // find callbacks for pin
                            for ( uint8_t j = 0; j < dev->pin_isr_cnt; j++)
                            {
                                if ( dev->pin_isr[ j ].pin == i )
                                {
                                    // check pin state
                                    uint8_t state = BIT_CHECK( data[1], i) != 0;

                                    if (( state == 1 && dev->pin_isr[ j ].intr_type == GPIO_INTR_POSEDGE) || 
                                        ( state == 0 && dev->pin_isr[ j ].intr_type == GPIO_INTR_NEGEDGE))
                                    {
                                        // execute callback
                                        dev->pin_isr[ j ].pin_cb( dev->pin_isr[ j ].args );
                                    }

                                }
                            }
                        }
                    }
				}
		}
		vTaskDelay( 10 / portTICK_PERIOD_MS );
	}
	vTaskDelete(NULL);
}
#endif

mcp23017_handle_t mcp23017_create(uint8_t addr)
{
    mcp23017_t* mcp23017 = (mcp23017_t*) calloc(1, sizeof(mcp23017_t));
    
    // TODO: load from nvs and use config in menuconfig
    mcp23017->status = MCP23017_DISABLED;
    mcp23017->addr = addr;
    mcp23017->pins_direction = MCP23017_GPIO_INPUTS_DEFAULT;
    mcp23017->pins_invert = 0;

    #ifdef CONFIG_MCP23017_ISR
	mcp23017->pins_interrupt = MCP23017_GPIO_INPUTS_DEFAULT;
	mcp23017->pins_def_val = MCP23017_GPIO_INPUTS_DEFAULT;
    #else
	mcp23017->pins_interrupt = 0;
	mcp23017->pins_def_val = 0;
    #endif

    mcp23017->pins_condition = 0;
    mcp23017->pins_saved = 0;
    mcp23017->pins_values = 0;

    #ifdef CONFIG_MCP23017_HTTP
    mcp23017->http_buttons = 0xFFFF;
    #endif

    // interrupts
    #ifdef CONFIG_MCP23017_ISR
 	mcp23017->taskq = NULL;
    mcp23017->task = NULL;
    mcp23017->isr_cb = mcp23017_isr_cb;
    mcp23017->int_a_pin = MCP23017_INTB_GPIO_DEFAULT;
    mcp23017->int_b_pin = MCP23017_INTA_GPIO_DEFAULT;
    #endif

    mcp23017_handle_t dev_h = (mcp23017_handle_t) mcp23017;
    mcp23017_enable( dev_h );



    return dev_h;
}


esp_err_t mcp23017_delete(mcp23017_handle_t dev_h)
{
    mcp23017_t* device = (mcp23017_t*) dev_h;
    device->i2c_bus_handle = NULL;
    free(device);
    return ESP_OK;
}

esp_err_t mcp23017_enable(mcp23017_handle_t dev_h)
{
    mcp23017_t * dev = (mcp23017_t *)dev_h;
    
    dev->i2c_bus_handle = i2c_bus_init();
    
    if ( dev->i2c_bus_handle == NULL ) return ESP_FAIL;

    mcp23017_set_directions( dev_h, dev->pins_direction);
    mcp23017_set_inversions(dev_h, dev->pins_invert);
    mcp23017_set_interrupts( dev_h, dev->pins_interrupt );
    mcp23017_set_defaults( dev_h, dev->pins_def_val);
    mcp23017_set_conditions( dev_h, dev->pins_condition);
    
    #ifdef CONFIG_MCP23017_ISR
    if ( dev->int_a_pin < 255 || dev->int_b_pin < 255 )
    {

    
		if ( !dev->task ) 
			xTaskCreate( dev->isr_cb, "mcp23017_isr", 1024, dev_h, 10, &dev->task);

		if ( !dev->taskq ) 
			dev->taskq = xQueueCreate(1, sizeof(uint16_t) * 2);

        // configure interrupts 
        gpio_config_t gpio_conf;
        gpio_conf.intr_type = GPIO_INTR_NEGEDGE; //GPIO_INTR_NEGEDGE; //GPIO_INTR_POSEDGE; // GPIO_INTR_ANYEDGE;           
        gpio_conf.mode = GPIO_MODE_INPUT;
        gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_conf.pull_up_en = GPIO_PULLUP_DISABLE;

        if ( dev->int_a_pin < 255 && dev->int_b_pin < 255 )
            gpio_conf.pin_bit_mask = ((1ULL << dev->int_a_pin) | (1ULL << dev->int_b_pin));
        else if ( dev->int_a_pin < 255 )    
            gpio_conf.pin_bit_mask = (1ULL << dev->int_a_pin);
        else if ( dev->int_b_pin < 255 )    
            gpio_conf.pin_bit_mask = (1ULL << dev->int_b_pin);

        gpio_config(&gpio_conf);    
        gpio_install_isr_service(0);
        
        if ( dev->int_a_pin < 255 ) 
            gpio_isr_handler_add( dev->int_a_pin, mcp23027_isr_handler, dev_h);    
        
        if ( dev->int_b_pin < 255 ) 
            gpio_isr_handler_add( dev->int_b_pin, mcp23027_isr_handler, dev_h);   
    } else {
        ESP_LOGE(TAG, "No gpio isr installed for mcp23017");
    }
    #endif

    dev->status = MCP23017_ENABLED;

    mcp23017_status_queue = xQueueCreate(5, sizeof(uint8_t) * 2);
    return ESP_OK;
}

esp_err_t mcp23017_disable(mcp23017_handle_t dev_h)
{
    mcp23017_t * mcp23017 = (mcp23017_t *)dev_h;
    mcp23017->i2c_bus_handle = NULL;
    mcp23017->status = MCP23017_DISABLED;
    return ESP_OK;
}

esp_err_t mcp23017_read_reg(mcp23017_handle_t dev_h, uint8_t reg_addr, uint8_t *value)
{
    if ( dev_h == NULL ) return ESP_FAIL;
    mcp23017_t *dev = (mcp23017_t *) dev_h;
    if ( xSemaphoreI2C == NULL ) return ESP_FAIL;
    if( xSemaphoreTake( xSemaphoreI2C, I2C_SEMAPHORE_WAIT ) == pdTRUE )
    {
        esp_err_t err = i2c_send_command( dev->addr, reg_addr);
        if ( err == ESP_FAIL ) goto error;

        err = i2c_read_data( dev->addr, value, 1);

    error:
        xSemaphoreGive( xSemaphoreI2C );
         return err;
    }
    return ESP_FAIL;            
}

esp_err_t mcp23017_write_reg(mcp23017_handle_t dev_h, uint8_t reg_addr, uint8_t value)
{
    if ( dev_h == NULL ) return ESP_FAIL;
    mcp23017_t *dev = (mcp23017_t *) dev_h;
    if ( xSemaphoreI2C == NULL ) return ESP_FAIL;
    if( xSemaphoreTake( xSemaphoreI2C, I2C_SEMAPHORE_WAIT ) == pdTRUE )
    {
        uint8_t data[2] = {reg_addr, value};
        esp_err_t err = i2c_write_data( dev->addr, (uint8_t *) &data, 2);
        xSemaphoreGive( xSemaphoreI2C );
        return err;
    }   
    return ESP_FAIL; 
}

esp_err_t mcp23017_read(mcp23017_handle_t dev_h, uint8_t reg_start, uint8_t reg_cnt, uint8_t *data )
{
    if ( dev_h == NULL ) return ESP_FAIL;
    mcp23017_t *dev = (mcp23017_t *) dev_h;
    if ( xSemaphoreI2C == NULL ) return ESP_FAIL;
    if( xSemaphoreTake( xSemaphoreI2C, I2C_SEMAPHORE_WAIT ) == pdTRUE )
    {
        esp_err_t err = i2c_send_command( dev->addr, reg_start);
        if ( err == ESP_FAIL ) goto error;

        err = i2c_read_data( dev->addr, data, reg_cnt);
    error:
        xSemaphoreGive( xSemaphoreI2C );
        return err;
    }   
    return ESP_FAIL;         
}

esp_err_t mcp23017_write(mcp23017_handle_t dev_h, uint8_t reg_addr, uint8_t reg_cnt, uint8_t *data)
{
    if ( dev_h == NULL ) return ESP_FAIL;

    for ( uint8_t i = 0; i < reg_cnt; i++)
    {
        if ( mcp23017_write_reg( dev_h, reg_addr + i, data[i]) != ESP_OK )
            return ESP_FAIL;
    }   
    return ESP_OK;         
}

esp_err_t mcp23017_set_interrupts(mcp23017_handle_t dev_h, uint16_t pins)
{
    if ( dev_h == NULL ) return ESP_FAIL;   

    uint8_t _pins[] = { MCP23017_PORT_A_BYTE(pins), MCP23017_PORT_B_BYTE(pins) };
    esp_err_t err = mcp23017_write( dev_h, MCP23017_REG_GPINTENA, sizeof(_pins), _pins);
    mcp23017_t *dev = (mcp23017_t *) dev_h;
    if ( err == ESP_OK ) 
    {
        // TODO: save to NVS    
        dev->pins_interrupt = pins;
    }
    return err;  
}

esp_err_t mcp23017_set_defaults(mcp23017_handle_t dev_h, uint16_t defaultValues)
{
    if ( dev_h == NULL ) return ESP_FAIL;   

    uint8_t _pins[] = { MCP23017_PORT_A_BYTE(defaultValues), MCP23017_PORT_B_BYTE(defaultValues) };
    esp_err_t err = mcp23017_write(dev_h, MCP23017_REG_DEFVALA, sizeof(_pins), _pins); 
    mcp23017_t *dev = (mcp23017_t *) dev_h;
    if ( err == ESP_OK ) 
    {
        // TODO: save to NVS    
        dev->pins_def_val = defaultValues;
    }
    return err;         
}

esp_err_t mcp23017_set_conditions(mcp23017_handle_t dev_h, uint16_t conditions)
{
    if ( dev_h == NULL ) return ESP_FAIL;   

    uint8_t _pins[] = { MCP23017_PORT_A_BYTE(conditions), MCP23017_PORT_B_BYTE(conditions) };
    esp_err_t err = mcp23017_write(dev_h, MCP23017_REG_INTCONA, sizeof(_pins), _pins);
    mcp23017_t *dev = (mcp23017_t *) dev_h;
    if ( err == ESP_OK ) 
    {
        // TODO: save to NVS    
        dev->pins_condition = conditions;
    }
    return err;        
}

esp_err_t mcp23017_set_directions(mcp23017_handle_t dev_h, uint16_t directions)
{
    if ( dev_h == NULL ) return ESP_FAIL;   

    uint8_t _pins[] = { MCP23017_PORT_A_BYTE(directions), MCP23017_PORT_B_BYTE(directions) };
    esp_err_t err = mcp23017_write(dev_h, MCP23017_REG_IODIRA, sizeof(_pins), _pins);    
    mcp23017_t *dev = (mcp23017_t *) dev_h;
    if ( err == ESP_OK ) 
    {
        // TODO: save to NVS    
        dev->pins_direction = directions;
    }
    return err;
}

esp_err_t mcp23017_set_inversions(mcp23017_handle_t dev_h, uint16_t pins)
{
    if ( dev_h == NULL ) return ESP_FAIL;   

    uint8_t _pins[] = { MCP23017_PORT_A_BYTE(pins), MCP23017_PORT_B_BYTE(pins) };
    esp_err_t err = mcp23017_write(dev_h, MCP23017_REG_IPOLA, sizeof(_pins), _pins);    
    mcp23017_t *dev = (mcp23017_t *) dev_h;
    if ( err == ESP_OK ) 
    {
        // TODO: save to NVS    
        dev->pins_invert = pins;
    }
    return err;
}

esp_err_t mcp23017_write_io(mcp23017_handle_t dev_h, uint16_t value)
{
    if ( dev_h == NULL ) return ESP_FAIL;   

    uint8_t _pins[] = { MCP23017_PORT_A_BYTE(value), MCP23017_PORT_B_BYTE(value) };
    esp_err_t err = mcp23017_write(dev_h, MCP23017_REG_GPIOA, sizeof(_pins), _pins);
          
    if ( err == ESP_OK ) {
        mcp23017_t *dev = (mcp23017_t *) dev_h;
        dev->pins_values = value;

        // TODO: save dev->pins_values () to NVS only for dev->pins_saved
    }
    return err;
}

esp_err_t mcp23017_read_io(mcp23017_handle_t dev_h, uint16_t *data)
{
    if ( dev_h == NULL ) return ESP_FAIL;   

    esp_err_t err = mcp23017_read(dev_h, MCP23017_REG_GPIOA, 2, data );
    if ( err == ESP_OK )
    {
        mcp23017_t *dev = (mcp23017_t *) dev_h;
        dev->pins_values = *data;
    }
    return err;
}

esp_err_t mcp23017_write_pin(mcp23017_handle_t dev_h, uint8_t pin, uint8_t val)
{
    if ( dev_h == NULL ) return ESP_FAIL;     

    uint8_t port = ( pin < 8 ) ? MCP23017_REG_GPIOA : MCP23017_REG_GPIOB;
    uint8_t value = 0;
    esp_err_t err = mcp23017_read_reg(dev_h, port, &value );
    if ( err == ESP_FAIL ) goto error;

    if ( val ) {
        BIT_SET( value, pin );
    } else {
        BIT_CLEAR( value, pin );
    }
    
    err = mcp23017_write_reg(dev_h, port, value );

    if ( err == ESP_OK ) {
        uint8_t data[2];
        data[0] = pin;
        data[1] = val;
        //ESP_LOGW(TAG, "put to queue: pin = %d, val = %d", data[0], data[1]);
        xQueueSendToBack(mcp23017_status_queue, &data, 0);
    }
error:
    return err;
}

esp_err_t mcp23017_read_pin(mcp23017_handle_t dev_h, uint8_t pin, uint8_t *val)
{
    //ESP_LOGI(TAG, "%s", __func__);
    if ( dev_h == NULL ) return ESP_FAIL;   

    uint8_t port = ( pin < 8 ) ? MCP23017_REG_GPIOA : MCP23017_REG_GPIOB;  

    uint8_t value = 0;
    esp_err_t err = mcp23017_read_reg(dev_h, port, &value ); 
    if ( err == ESP_FAIL ) goto error;

    *val = BIT_CHECK(value, pin);

error:
    return err;   
}

#ifdef CONFIG_MCP23017_ISR
esp_err_t mcp23017_isr_handler_add(mcp23017_handle_t dev_h, uint8_t pin, gpio_int_type_t intr_type, interrupt_cb cb, void *args)
{
    if ( cb == NULL ) return ESP_FAIL;
    if ( dev_h == NULL ) return ESP_FAIL; 
    mcp23017_t *dev = (mcp23017_t *) dev_h;

    // search aded callback
    for (uint8_t i=0; i < dev->pin_isr_cnt; i++ )
    {
        if ( dev->pin_isr[i].pin_cb == cb && dev->pin_isr[i].pin == pin)
        {
            ESP_LOGE(TAG, "Callback already registered for pin %d", pin);
            return ESP_FAIL;
        }
    }
    
    dev->pin_isr_cnt++;
    dev->pin_isr = (mcp23017_pin_isr_t *) realloc(dev->pin_isr, dev->pin_isr_cnt * sizeof(mcp23017_pin_isr_t));
    if ( dev->pin_isr == NULL ) {
        dev->pin_isr_cnt--;
        return ESP_FAIL;
    }

    dev->pin_isr[ dev->pin_isr_cnt-1 ].pin = pin;
    dev->pin_isr[ dev->pin_isr_cnt-1 ].pin_cb = cb;
    dev->pin_isr[ dev->pin_isr_cnt-1 ].intr_type = intr_type;
    dev->pin_isr[ dev->pin_isr_cnt-1 ].args = args;
    return ESP_OK;
}
#endif

#endif