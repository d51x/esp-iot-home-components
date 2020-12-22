
#include "pzem004t.h"
#include "utils.h"
#include "freertos/task.h"

#include "iot_debug.h"

#ifdef CONFIG_SENSORS_GET
#include "sensors.h"
const char *pzem_sensors_data ICACHE_RODATA_ATTR = "pmv:%0.2f;pmc:%0.2f;pmw:%d;pmwh:%d;";
#endif



#ifdef CONFIG_SENSOR_PZEM004_T

static const char *TAG = "PZEM";

#define UART_READ_TIMEOUT					1500  // влияет на результаты чтения из юсарт
#define PZEM_PAUSE_TASK 	20

#define VOLTAGE_TRESHOLD 400
#define CURRENT_TRESHOLD 100
#define POWER_TRESHOLD 25000

#define CMD_VOLTAGE 		0xB0
#define RESP_VOLTAGE 		0xA0
#define CMD_CURRENT 		0xB1
#define RESP_CURRENT 		0xA1
#define CMD_POWER   		0xB2
#define RESP_POWER   		0xA2
#define CMD_ENERGY  		0xB3
#define RESP_ENERGY  		0xA3
#define CMD_ADDRESS         0xB4
#define RESP_ADDRESS        0xA4

#define RX_BUF_SIZE UART_FIFO_LEN + 1
#define TX_BUF_SIZE UART_FIFO_LEN + 1

#define PZEM_PERIODIC_TASK_PRIORITY 13
#define PZEM_ENERGY_TASK_PRIORITY 14
#define PZEM_PERIODIC_TASK_STACK_DEPTH 1024 //+ 256 //512
#define PZEM_ENERGY_TASK_STACK_DEPTH 1024*2

#ifdef CONFIG_SENSOR_PZEM004_T_CALC_CONSUMPTION
	#define PZEM_ENERGY_PERIODIC_DELAY 1 // sec
	//#define PZEM_NVS_SECTION "pzem"
	//#define PZEM_NVS_PARAM_ENERGY "energy"

	static RTC_NOINIT_ATTR  pzem_energy_t rtc_pzem_energy;
	static RTC_NOINIT_ATTR  uint8_t rtc_pzem_energy_crc;	
	static RTC_NOINIT_ATTR  float rtc_pzem_energy_counter;	
#endif

#ifdef CONFIG_SENSOR_PZEM004_T_SOFTUART
uint8_t _uart_num = 0;
#else
uart_port_t _uart_num = UART_NUM_0;
#endif

TaskHandle_t xPzemHandle = NULL;

PZEM_Address _pzem_addr = {192, 168, 1, 1};

typedef struct {
    uint8_t command;
    //uint8_t addr[4];
    PZEM_Address addr;
    uint8_t data;
    uint8_t crc;
} PZEM_Command_t;

#define RESPONSE_SIZE sizeof(PZEM_Command_t)
#define RESPONSE_DATA_SIZE RESPONSE_SIZE - 2

static pzem_data_t _pzem_data;

pzem_read_strategy_t _strategy;

#define PZEM_READ_ERROR_COUNT 20

static uint8_t pzem_crc(const uint8_t *data, uint8_t sz);
#ifdef CONFIG_SENSORS_GET
static void pzem_sensors_print(char **buf, void *args);
#endif

// #ifdef CONFIG_SENSOR_PZEM004_T_CALC_CONSUMPTION
// static void pzem_nvs_save()
// {
// 	nvs_param_save(PZEM_NVS_SECTION, PZEM_NVS_PARAM_ENERGY, &_pzem_data.energy_values, sizeof(pzem_energy_t));
// }
// #endif

static void pzem_energy_values_save()
{
	memcpy(&rtc_pzem_energy, &_pzem_data.energy_values, sizeof(pzem_energy_t));
	rtc_pzem_energy_crc = pzem_crc((uint8_t *)&rtc_pzem_energy, sizeof(pzem_energy_t) );	
}

//UART_NUM_0
void pzem_init(uint8_t uart_num)
{

    _uart_num = uart_num;

	#ifdef CONFIG_SENSOR_PZEM004_T_SOFTUART
	softuart_open(_uart_num, 9600, 2 /*RX*/, 0 /*TX*/, UART_READ_TIMEOUT);
	#else
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
	
    uart_param_config( _uart_num, &uart_config );
	uart_set_baudrate( _uart_num, 9600);
	uint32_t br;
	uart_get_baudrate( _uart_num, &br);
		#ifdef CONFIG_DEBUG_UART1
		userlog("%s: baudrate %d\n", __func__, br);
		#endif
		ESP_LOGW(TAG, "%s: baudrate %d\n", __func__, br);

    uart_driver_install(_uart_num, RX_BUF_SIZE, TX_BUF_SIZE, 10, NULL, 0); 
	#endif
 
    memset(&_pzem_data, 0, sizeof(pzem_data_t));  

	_strategy.voltage_read_count = 1;
	_strategy.current_read_count = 1;
	_strategy.power_read_count = 1;
	_strategy.energy_read_count = 1;
	_pzem_data.ready = ESP_FAIL;

	// reset reason
    esp_reset_reason_t reason = esp_reset_reason();
    ESP_LOGW(TAG, "Reset reason: %d (0x%02X) %s"
                , reason, reason, RESET_REASONS[reason]);

	
	#ifdef CONFIG_SENSOR_PZEM004_T_CALC_CONSUMPTION

	if ( rtc_pzem_energy_crc != pzem_crc((uint8_t *)&rtc_pzem_energy, sizeof(pzem_energy_t)) )
	{
		// first start
		ESP_LOGE(TAG, "Pzem rtc mem data wrong (crc incorrect)");
		memset(&_pzem_data.energy_values, 0, sizeof(pzem_energy_t));
		pzem_energy_values_save();	
		rtc_pzem_energy_counter = 0;
	} else {
		ESP_LOGW(TAG, "prev_midnight = %d", rtc_pzem_energy.prev_midnight);
		ESP_LOGW(TAG, "today_midnight = %d", rtc_pzem_energy.today_midnight);
		ESP_LOGW(TAG, "prev_t1 = %d", rtc_pzem_energy.prev_t1);
		ESP_LOGW(TAG, "prev_t2 = %d", rtc_pzem_energy.prev_t2);
		ESP_LOGW(TAG, "today_t1 = %d", rtc_pzem_energy.today_t1);
		ESP_LOGW(TAG, "today_t2 = %d", rtc_pzem_energy.today_t2);
		memcpy(&_pzem_data.energy_values, &rtc_pzem_energy, sizeof(pzem_energy_t));
		_pzem_data.energy = rtc_pzem_energy_counter;
	}
	//if ( nvs_param_load(PZEM_NVS_SECTION, PZEM_NVS_PARAM_ENERGY, &_pzem_data.energy_values) != ESP_OK )
	//{
	//	memset(&_pzem_data.energy_values, 0, sizeof(pzem_energy_t));
	//	pzem_nvs_save();
	//}
	#endif

    #ifdef CONFIG_SENSORS_GET
    sensors_add("pzem", pzem_sensors_print, NULL); 
    #endif
}

static void pzem_send_buffer(const uint8_t *buffer, uint8_t len)
{
    
	#ifdef CONFIG_DEBUG_UART1
		//userlog("%08d > %s: len %d\n", millis(), __func__, len);
		if ( len > 0 ) {
			userlog("%s: buf = ", __func__);
			for (uint8_t i = 0; i < len; i++) {
				userlog("%02X ", buffer[i]);
			}
			userlog("\n");
		}
	#endif

	#ifdef CONFIG_SENSOR_PZEM004_T_SOFTUART
	softuart_write_bytes(_uart_num, buffer, len);
	#else
	uart_write_bytes(_uart_num, (const char *) buffer, len);
	#endif
}

static uint8_t pzem_read_buffer(uint8_t *buffer, uint8_t cnt)
{
	int8_t result = 0;

	#ifdef CONFIG_SENSOR_PZEM004_T_SOFTUART
	result = softuart_read_buf(_uart_num, (char *)buffer, cnt);	// TOSO   add timeout for reading
	#else
	result = uart_read_bytes(_uart_num, buffer, cnt, UART_READ_TIMEOUT / portTICK_RATE_MS );
	#endif

	#ifdef CONFIG_DEBUG_UART1
		userlog("%08d > %s: len %d\n", millis(), __func__, result);
	#endif	
	if 	( result < 0 ) { result = 0;	}
	return result;
}

static uint8_t pzem_crc(const uint8_t *data, uint8_t sz)
{
    uint16_t crc = 0;
    for(uint8_t i=0; i<sz; i++)
        crc += *data++;
    return (uint8_t)(crc & 0xFF);
}

static void pzem_send (uint8_t *addr, uint8_t cmd)
{
	PZEM_Command_t pzem;
	pzem.command = cmd;
	for ( uint8_t i = 0; i < sizeof(pzem.addr); i++) pzem.addr[i] = addr[i];
    //memcpy(pzem.addr, addr, sizeof(PZEM_Address));
	pzem.data = 0;
	uint8_t *bytes = (uint8_t*)&pzem;
	pzem.crc = pzem_crc(bytes, sizeof(PZEM_Command_t) - 1);
	//pzem.crc = pzem_crc((uint8_t*)&pzem, sizeof(PZEM_Command_t) - 1);

		// char wlog[128];
		// sprintf(wlog, "%s: len %d buf: ", __func__, sizeof(PZEM_Command_t));
		// char r[8];  
		// for (uint8_t i = 0; i < sizeof(PZEM_Command_t); i++) {
		// 	sprintf(r, "%02X ", bytes[i]);
		// 	strcat(wlog + strlen(wlog), r);
		// }
		// ESP_LOGW(TAG, wlog);  

	pzem_send_buffer(bytes, sizeof(PZEM_Command_t));
	//send_buffer((uint8_t*)&pzem, sizeof(PZEM_Command_t));
}

static esp_err_t pzem_read(uint8_t resp, uint8_t *data)
{
	esp_err_t res = ESP_FAIL;
	uint8_t *buf = (uint8_t *) malloc(RESPONSE_SIZE);
	uint8_t len = pzem_read_buffer(buf, RESPONSE_SIZE);
	

	#ifdef CONFIG_DEBUG_UART1
		userlog("%08d > %s: len %d\n", millis(), __func__, len);
	  	if ( len > 0 ) {
	  		userlog("%s: len: %d \t buf = ", __func__, len);
	  		for (uint8_t i = 0; i < len; i++) {
	  			userlog("%02X ", buf[i]);
	  		}
	  		userlog("\n");
	  	}
	#else
		// char wlog[128];
		// sprintf(wlog, "%s: len %d", __func__, len);
	  	// if ( len > 0 ) 
		// {
		// 	strcat(wlog + strlen(wlog), " buf: ");
		// 	char r[8];  
	  	// 	for (uint8_t i = 0; i < len; i++) {
	  	// 		sprintf(r, "%02X ", buf[i]);
		// 		strcat(wlog + strlen(wlog), r);
	  	// 	}
	  	// }		  
		// ESP_LOGW(TAG, "%s", wlog);  
		// ESP_LOGW(TAG, " ");  
	#endif

	if ( len == 0 ) res = ESP_FAIL;
	else 
		if ( len ==  RESPONSE_SIZE 
			&& buf[0] == resp 
			&& buf[6] == pzem_crc(buf, len-1)
			) 
		{
			for ( uint8_t i = 0; i < RESPONSE_DATA_SIZE; i++) data[i] = buf[1 + i];
			res = ESP_OK;
		}	
	free(buf);
	buf = NULL;
	return res;
}

static float pzem_voltage(uint8_t *addr) {
	uint8_t data[RESPONSE_DATA_SIZE];
	pzem_send(addr, CMD_VOLTAGE);
	esp_err_t err = pzem_read( RESP_VOLTAGE, data);
	//pauseTask(10);
	float value = (err == ESP_OK ) ? (data[0] << 8) + data[1] + ( data[2] / 10.0) : PZEM_ERROR_VALUE;
	return value;
}

static float pzem_current(uint8_t *addr) {
	uint8_t data[RESPONSE_DATA_SIZE];
	pzem_send(addr, CMD_CURRENT);	
	esp_err_t err = pzem_read( RESP_CURRENT, data);
	//pauseTask(10);
	float value = (err == ESP_OK ) ? (data[0] << 8) + data[1] + (data[2] / 100.0) : PZEM_ERROR_VALUE;
	return value;
}

static float pzem_power(uint8_t *addr) {
	uint8_t data[RESPONSE_DATA_SIZE];
	pzem_send(addr, CMD_POWER);
	esp_err_t err = pzem_read( RESP_POWER, data);
	//pauseTask(10);
	float value = (err == ESP_OK) ? (data[0] << 8) + data[1] : PZEM_ERROR_VALUE;
	return value;
}

static float pzem_energy(uint8_t *addr) {
	uint8_t data[RESPONSE_DATA_SIZE];
	pzem_send(addr, CMD_ENERGY);
	esp_err_t err = pzem_read( RESP_ENERGY, data);
	//pauseTask(10);
	float value = (err == ESP_OK ) ? ((uint32_t)data[0] << 16) + ((uint16_t)data[1] << 8) + data[2] : PZEM_ERROR_VALUE;
	return value;
}

esp_err_t pzem_set_addr(PZEM_Address *_addr)
{
	//ESP_LOGW(TAG, __func__);	
	memcpy(&_pzem_addr, _addr, 4);
    uint8_t data[RESPONSE_DATA_SIZE];
    pzem_send(_pzem_addr, CMD_ADDRESS);
    esp_err_t err = pzem_read(RESP_ADDRESS, data);

	#ifdef CONFIG_DEBUG_UART1
		userlog("%s result %s \n", __func__, esp_err_to_name(err) );
	#endif

    //pauseTask(10);
    return err;
}

float pzem_read_voltage()
{
	//ESP_LOGW(TAG, __func__);
	#ifdef CONFIG_COMPONENT_DEBUG
	log_rtc_debug_str("pzem_read_voltage");
	#endif

	float v = pzem_voltage(_pzem_addr);
	_pzem_data.voltage = ( v == 0 || v > VOLTAGE_TRESHOLD) ? _pzem_data.voltage : v;

	// #ifdef CONFIG_DEBUG_UART1
	// 	userlog("%08d > voltage \t %0.2f \n", millis(), _pzem_data.voltage);
	// #endif

	if ( v == 0 ) _pzem_data.errors++;
    //pauseTask(PZEM_PAUSE_TASK );
    return _pzem_data.voltage;
}

float pzem_read_current()
{
	//ESP_LOGW(TAG, __func__);
	#ifdef CONFIG_COMPONENT_DEBUG
	log_rtc_debug_str("pzem_read_current");
	#endif 

	float v = pzem_current(_pzem_addr);
	_pzem_data.current = ( v == 0 || v > CURRENT_TRESHOLD) ? _pzem_data.current : v;

	// #ifdef CONFIG_DEBUG_UART1
	// 	userlog("%08d > current \t %0.2f \n", millis(), _pzem_data.current);
	// #endif

    //pauseTask(PZEM_PAUSE_TASK );
    return _pzem_data.current;    
}

float pzem_read_power()
{
	//ESP_LOGW(TAG, __func__);
	#ifdef CONFIG_COMPONENT_DEBUG
	log_rtc_debug_str("pzem_read_power");
	#endif

	float v = pzem_power(_pzem_addr);
	_pzem_data.power = ( v == 0 || v > POWER_TRESHOLD) ? _pzem_data.power : v;	

	// #ifdef CONFIG_DEBUG_UART1
	// 	userlog("%08d > power \t %0.2f \n", millis(), _pzem_data.power);
	// #endif

    //pauseTask(PZEM_PAUSE_TASK );
    return _pzem_data.power;        
}

float pzem_read_energy()
{
	//ESP_LOGW(TAG, __func__);
	#ifdef CONFIG_COMPONENT_DEBUG
	log_rtc_debug_str("pzem_read_energy");
	#endif

	float v = pzem_energy(_pzem_addr);	
	_pzem_data.energy = ( v == 0) ? _pzem_data.energy : v;

	// #ifdef CONFIG_DEBUG_UART1
	// 	userlog("%08d > energy \t %0.2f \n", millis(), _pzem_data.energy);
	// #endif

    //pauseTask(PZEM_PAUSE_TASK );
    return _pzem_data.energy;     
}

pzem_data_t pzem_get_data()
{
	return _pzem_data;
}

void pzem_reset_consumption(bool today)
{
	if ( today ) 
	{
		_pzem_data.energy_values.today_midnight = 0;
		_pzem_data.energy_values.today_t1 = 0;
		_pzem_data.energy_values.today_t2 = 0;
	}

	_pzem_data.energy_values.prev_midnight = 0;
	_pzem_data.energy_values.prev_t1 = 0;
	_pzem_data.energy_values.prev_t2 = 0;

	//pzem_nvs_save();
	// memset(&rtc_pzem_energy, 0, sizeof(pzem_energy_t));
	// rtc_pzem_energy_crc = pzem_crc(&rtc_pzem_energy, sizeof(pzem_energy_t) );	
	pzem_energy_values_save();
}

void pzem_set_read_strategy(pzem_read_strategy_t strategy)
{
	_strategy = strategy;
}

#ifdef CONFIG_SENSOR_PZEM004_T_CALC_CONSUMPTION
static void calc_energy_values(void *arg)
{
	uint32_t delay = (uint32_t)arg;
	uint32_t energy;
	struct tm timeinfo;

	// task started 
    while (1) 
	{
		energy = (uint32_t)_pzem_data.energy;
		//ESP_LOGW(TAG, "%s: %d (%0.2f)", __func__, energy, _pzem_data.energy);
		if ( _pzem_data.energy_values.today_midnight == 0) _pzem_data.energy_values.today_midnight = energy;
		if ( _pzem_data.energy_values.today_t1 == 0) _pzem_data.energy_values.today_t1 = energy;
		if ( _pzem_data.energy_values.today_t2 == 0) _pzem_data.energy_values.today_t2 = energy;

		if ( _pzem_data.energy_values.prev_midnight == 0) _pzem_data.energy_values.prev_midnight = energy;
		if ( _pzem_data.energy_values.prev_t1 == 0) _pzem_data.energy_values.prev_t1 = energy;
		if ( _pzem_data.energy_values.prev_t2 == 0) _pzem_data.energy_values.prev_t2 = energy;

		// store energy values to RTC MEM
		pzem_energy_values_save();

		get_timeinfo(&timeinfo);

		// проверить, что время установилось
		if ( timeinfo.tm_year <  (2016 - 1900) ) 
		{
			//ESP_LOGE(TAG, "Time is not set!");
			goto loop_end;
		}


		// в полночь обнуляем счетчик
		if ( timeinfo.tm_hour == 0 && timeinfo.tm_min ==0 && timeinfo.tm_sec == 0 
			&& timeinfo.tm_year >  (2016 - 1900) // реальная дата
			) 
		{
			ESP_LOGW(TAG, "Reset today energy values, time is 00:00:00");
			_pzem_data.energy_values.prev_midnight = _pzem_data.energy_values.today_midnight; 
			_pzem_data.energy_values.today_midnight = energy;
			_pzem_data.energy_values.prev_t1 = _pzem_data.energy_values.today_t1;
			_pzem_data.energy_values.prev_t2 = _pzem_data.energy_values.today_t2;
			//pzem_nvs_save();
		}
		// в 7 утра
		else if ( timeinfo.tm_hour == PZEM_ENERGY_ZONE_T1_HOUR && timeinfo.tm_min ==0 && timeinfo.tm_sec == 0 )
		{
			ESP_LOGW(TAG, "Store t1 values, time is 07:00:00");
			_pzem_data.energy_values.prev_t1 = _pzem_data.energy_values.today_t1;
			_pzem_data.energy_values.today_t1 = energy;
			//pzem_nvs_save();
		}
		// в 23 вечера
		else if ( timeinfo.tm_hour == PZEM_ENERGY_ZONE_T2_HOUR && timeinfo.tm_min ==0 && timeinfo.tm_sec == 0 )
		{
			ESP_LOGW(TAG, "Store t2 values, time is 23:00:00");
			_pzem_data.energy_values.prev_t2 = _pzem_data.energy_values.today_t2;
			_pzem_data.energy_values.today_t2 = energy;		
			//pzem_nvs_save();
		}

		// TODO: сохраняем значения только в определенный час, если в этот момент esp ребутнулось, то данные по реальному расходу будут не актуальные
		// вчера: midnight --> t1 --> t2 --> сегодня : midnight --> t1 --> t2
		if ( _pzem_data.energy_values.prev_t1 < _pzem_data.energy_values.prev_midnight) _pzem_data.energy_values.prev_t1 = _pzem_data.energy_values.prev_midnight;
		if ( _pzem_data.energy_values.prev_t2 < _pzem_data.energy_values.prev_midnight) _pzem_data.energy_values.prev_t2 = _pzem_data.energy_values.prev_midnight;
		if ( _pzem_data.energy_values.prev_t2 < _pzem_data.energy_values.prev_t1) _pzem_data.energy_values.prev_t2 = _pzem_data.energy_values.prev_t1;
		//if ( _pzem_data.energy_values.today_midnight < _pzem_data.energy_values.prev_t2) _pzem_data.energy_values.prev_t2 = _pzem_data.energy_values.today_midnight;

		// расчет реального потребления
		// за сегодня (полный день, до текущего момента)
		_pzem_data.consumption.today_total = energy - _pzem_data.energy_values.today_midnight;

		// за вчера (полный день)
		if ( _pzem_data.energy_values.today_midnight <= _pzem_data.energy_values.prev_midnight)
			_pzem_data.consumption.prev_total = 0;
		else
			_pzem_data.consumption.prev_total = _pzem_data.energy_values.today_midnight - _pzem_data.energy_values.prev_midnight;

		//предыдущая ночь 0:00 - 7:00 + 23:00 - 23:59:59
		_pzem_data.consumption.prev_night = _pzem_data.energy_values.prev_t1 - _pzem_data.energy_values.prev_midnight +   // с 00 до 07
                         					_pzem_data.energy_values.today_midnight - _pzem_data.energy_values.prev_t2;   // с 23 до 00

		// предыдущий день 7:00 - 23:00
		if ( _pzem_data.energy_values.prev_t2 > _pzem_data.energy_values.prev_t1)
			_pzem_data.consumption.prev_day = _pzem_data.energy_values.prev_t2 - _pzem_data.energy_values.prev_t1;
		else	
			_pzem_data.consumption.prev_day = 0;

		// сегодня ночью 0:00 - 7:00
		if ( timeinfo.tm_hour < PZEM_ENERGY_ZONE_T1_HOUR && timeinfo.tm_year >  (2016 - 1900) )
		{
			_pzem_data.consumption.today_night = energy - _pzem_data.energy_values.today_midnight;
			_pzem_data.consumption.today_day = 0;
		}
		else if ( timeinfo.tm_hour < PZEM_ENERGY_ZONE_T2_HOUR && timeinfo.tm_year >  (2016 - 1900) )
		{
			_pzem_data.consumption.today_night = _pzem_data.energy_values.today_t1  - _pzem_data.energy_values.today_midnight;
			_pzem_data.consumption.today_day = energy - _pzem_data.energy_values.today_t1;
		}
		else if ( timeinfo.tm_hour >= PZEM_ENERGY_ZONE_T2_HOUR && timeinfo.tm_year >  (2016 - 1900))
		{
			_pzem_data.consumption.today_night = _pzem_data.energy_values.today_t1  - _pzem_data.energy_values.today_midnight + energy - _pzem_data.energy_values.today_t2;
			_pzem_data.consumption.today_day = _pzem_data.energy_values.today_t2 - _pzem_data.energy_values.today_t1;
		}


	loop_end:
		pauseTask(delay * 1000);
	}

 	vTaskDelete( NULL );
}
#endif

static void pzem_periodic_task(void *arg)
{
	uint32_t delay = (uint32_t)arg;
	uint32_t i;

	_pzem_data.errors = 0;

    while (1) 
	{	

		#ifdef CONFIG_DEBUG_UART1
			userlog("%s result %s \n", __func__, esp_err_to_name(_pzem_data.ready) );
		#endif

		// debug
		//_pzem_data.energy += 100;
		//rtc_pzem_energy_counter = _pzem_data.energy;

		if ( _pzem_data.ready != ESP_OK ) 
		{
			#ifdef CONFIG_COMPONENT_DEBUG
			log_rtc_debug_str("try to set pzem address");
			#endif
			
			#ifdef CONFIG_DEBUG_UART1
				userlog("try to set pzem address \n");
			#endif			
			_pzem_data.ready = pzem_set_addr(&_pzem_addr);
			pauseTask(1000);
		} 
		else 
		{
			#ifdef CONFIG_DEBUG_UART1
			userlog("get pzem data\n");
			#endif

			for (i = 0; i < _strategy.voltage_read_count; i++)
				pzem_read_voltage();

			for (i = 0; i < _strategy.current_read_count; i++)	
			 	pzem_read_current();

			for (i = 0; i < _strategy.power_read_count; i++)	
			 	pzem_read_power();

			for (i = 0; i < _strategy.energy_read_count; i++)	
			 	pzem_read_energy();

			if ( _pzem_data.errors >= PZEM_READ_ERROR_COUNT )
			{
				pzem_set_addr(&_pzem_addr);
				//esp_restart();
				pauseTask(1000);
			}
			pauseTask(delay * 1000);
			//pauseTask(1000);
		}
    }

    vTaskDelete( NULL );	
}

void pzem_task_start(uint32_t delay_sec)
{
	xTaskCreate(pzem_periodic_task, "pzem_task", PZEM_PERIODIC_TASK_STACK_DEPTH, delay_sec, PZEM_PERIODIC_TASK_PRIORITY, &xPzemHandle);
	#ifdef CONFIG_SENSOR_PZEM004_T_CALC_CONSUMPTION
	xTaskCreate(calc_energy_values, "pzem_enrg", PZEM_ENERGY_TASK_STACK_DEPTH, PZEM_ENERGY_PERIODIC_DELAY, PZEM_ENERGY_TASK_PRIORITY, NULL);
	#endif
}

#ifdef CONFIG_SENSORS_GET
static void pzem_sensors_print(char **buf, void *args)
{
    size_t sz = get_buf_size(pzem_sensors_data
							, _pzem_data.voltage
							, _pzem_data.current
							, (uint32_t)_pzem_data.power
							, (uint32_t)_pzem_data.energy
							);
    *buf = (char *) realloc(*buf, sz+1);
    memset(*buf, 0, sz+1);
    snprintf(*buf, sz+1, pzem_sensors_data
							, _pzem_data.voltage
							, _pzem_data.current
							, (uint32_t)_pzem_data.power
							, (uint32_t)_pzem_data.energy
							);
}
#endif

#endif