

#include "driver/gpio.h"
#include "onewire.h"



static const char *TAG = "ONEWIRE";
static uint16_t oddparity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

// global search state
static unsigned char address[8];
static uint8_t LastDiscrepancy;
static uint8_t LastFamilyDiscrepancy;
static uint8_t LastDeviceFlag;


static inline esp_err_t _onewire_wait_for_bus(int pin, int max_wait) {
    esp_err_t state;
    for (int i = 0; i < ((max_wait + 4) / 5); i++) {
        if (gpio_get_level(pin)) break;
        os_delay_us(5);
    }
    state = gpio_get_level(pin);
    // Wait an extra 1us to make sure the devices have an adequate recovery
    // time before we drive things low again.
    os_delay_us(2);
    return state;
}

// Perform the onewire reset function.  We will wait up to 250uS for
// the bus to come high, if it doesn't then it is broken or shorted
// and we return a 0;
// Returns 1 if a device asserted a presence pulse, 0 otherwise.
esp_err_t ICACHE_FLASH_ATTR onewire_reset(uint8_t pin)
{
	esp_err_t r;
	uint8_t retries = 125;
	
    gpio_set_direction(pin, GPIO_MODE_INPUT);
	
	do {
		if (--retries == 0) return ESP_FAIL;
		os_delay_us(2);
	//} while ( !GPIO_INPUT_GET(DS18B20_PIN));
	} while ( !gpio_get_level(pin));
	

	//if (!_onewire_wait_for_bus(pin, 250)) return ESP_FAIL;
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
	//etc_delay_us(500);
	ets_delay_us(480);
	//GPIO_DIS_OUTPUT(DS18B20_PIN);

    gpio_set_direction(pin, GPIO_MODE_INPUT);
	//os_delay_us(65);
	ets_delay_us(70);
	//r = !GPIO_INPUT_GET(DS18B20_PIN);
	r = !gpio_get_level(pin);
	//os_delay_us(490);
	
	ets_delay_us(410);
	//if (!_onewire_wait_for_bus(pin, 410)) return ESP_FAIL;

	return r;
}

//
// Write a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
static esp_err_t ICACHE_FLASH_ATTR onewire_write_bit(uint8_t pin, uint8_t v)
{
	//if (!_onewire_wait_for_bus(pin, 10)) return ESP_FAIL;
	//GPIO_OUTPUT_SET(DS18B20_PIN, 0);
	taskENTER_CRITICAL();
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
	if(v & 1) {
		os_delay_us(10);
	
        gpio_set_direction(pin, GPIO_MODE_OUTPUT);
        gpio_set_level(pin, 1);
		taskEXIT_CRITICAL();
		os_delay_us(55);
	} else {
		os_delay_us(65);
		
        gpio_set_direction(pin, GPIO_MODE_OUTPUT);
        gpio_set_level(pin, 1);
		taskEXIT_CRITICAL();
		os_delay_us(5);
	}
	return ESP_OK;
}

//
// Read a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
static uint8_t ICACHE_FLASH_ATTR onewire_read_bit(uint8_t pin)
{
	uint8_t r;
	//if (!_onewire_wait_for_bus(pin, 10)) return ESP_FAIL;
	taskENTER_CRITICAL();
	gpio_set_direction(pin, GPIO_MODE_OUTPUT);
	gpio_set_level(pin, 0);    
	os_delay_us(3);
	
    gpio_set_direction(pin, GPIO_MODE_INPUT);
	os_delay_us(10);
	
	r = gpio_get_level(pin);
	taskEXIT_CRITICAL();
	os_delay_us(53);
	return r;
}

// Write a byte. The writing code uses the active drivers to raise the
// pin high, if you need power after the write (e.g. DS18S20 in
// parasite power mode) then set 'power' to 1, otherwise the pin will
// go tri-state at the end of the write to avoid heating in a short or
// other mishap.
//
esp_err_t ICACHE_FLASH_ATTR onewire_write(uint8_t pin, uint8_t v, int power)
{
	uint8_t bitMask;
	for (bitMask = 0x01; bitMask; bitMask <<= 1) {
		if ( onewire_write_bit(pin, (bitMask & v)?1:0) == ESP_FAIL)
			return ESP_FAIL;
	}
	if (!power) {
		//GPIO_DIS_OUTPUT(DS18B20_PIN);
        gpio_set_direction(pin, GPIO_MODE_INPUT);
		//GPIO_OUTPUT_SET(DS18B20_PIN, 0);
        gpio_set_direction(pin, GPIO_MODE_OUTPUT);
        gpio_set_level(pin, 0);
	}
	return ESP_OK;
}

void onewire_reset_search() {
	int i;
	// reset the search state
	LastDiscrepancy = 0;
	LastDeviceFlag = 0;
	LastFamilyDiscrepancy = 0;
	for(i = 7; ; i--) {
		address[i] = 0;
		if ( i == 0) break;
	}    
}

/* pass array of 8 bytes in */
esp_err_t ICACHE_FLASH_ATTR onewire_search(uint8_t pin, uint8_t *newAddr) {
	uint8_t id_bit_number = 1;
	uint8_t last_zero = 0;
	uint8_t rom_byte_number = 0;
	uint8_t id_bit, cmp_id_bit;
	esp_err_t search_result = ESP_FAIL;
	int i;

	unsigned char rom_byte_mask = 1;
	unsigned char search_direction;

    // if the last call was not the last one
	if (!LastDeviceFlag)
	{
		// 1-Wire reset
		if (!onewire_reset(pin))
		{
			// reset the search
			LastDiscrepancy = 0;
			LastDeviceFlag = 0;
			LastFamilyDiscrepancy = 0;
			return ESP_FAIL;
		}

		// issue the search command
		onewire_write(pin, ONEWIRE_SEARCHROM, 0);

		// loop to do the search
		do
		{
			// read a bit and its complement
			id_bit = onewire_read_bit(pin);
			cmp_id_bit = onewire_read_bit(pin);
	 
			// check for no devices on 1-wire
			if ((id_bit == 1) && (cmp_id_bit == 1))
				break;
			else
			{
				// all devices coupled have 0 or 1
				if (id_bit != cmp_id_bit)
					search_direction = id_bit;  // bit write value for search
				else
				{
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if (id_bit_number < LastDiscrepancy)
						search_direction = ((address[rom_byte_number] & rom_byte_mask) > 0);
					else
						// if equal to last pick 1, if not then pick 0
						search_direction = (id_bit_number == LastDiscrepancy);

					// if 0 was picked then record its position in LastZero
					if (search_direction == 0)
					{
						last_zero = id_bit_number;

						// check for Last discrepancy in family
						if (last_zero < 9)
							LastFamilyDiscrepancy = last_zero;
					}
				}

				// set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if (search_direction == 1)
					address[rom_byte_number] |= rom_byte_mask;
				else
					address[rom_byte_number] &= ~rom_byte_mask;

				// serial number search direction write bit
				onewire_write_bit(pin, search_direction);

				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1;

				// if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
				if (rom_byte_mask == 0)
				{
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		}
		while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

		// if the search was successful then
		if (!(id_bit_number < 65))
		{
			// search successful so set LastDiscrepancy,LastDeviceFlag,search_result
			LastDiscrepancy = last_zero;

			// check for last device
			if (LastDiscrepancy == 0)
				LastDeviceFlag = 1;

			search_result = ESP_OK;
		}
	}

	// if no device found then reset counters so next 'search' will be like a first
	if (search_result == ESP_FAIL || !address[0])
	{
		LastDiscrepancy = 0;
		LastDeviceFlag = 0;
		LastFamilyDiscrepancy = 0;
		search_result = ESP_FAIL;
	} else {
		for (i = 0; i < 8; i++) newAddr[i] = address[i];
	}
	return search_result;    
}

//
// Do a ROM select
//
void ICACHE_FLASH_ATTR onewire_select(uint8_t pin, const uint8_t *rom)
{
	uint8_t i;
	onewire_write(pin, ONEWIRE_MATCHROM, 0); // Choose ROM
	for (i = 0; i < 8; i++) onewire_write(pin, rom[i], 0);
}

//
// Do a ROM skip
//
void ICACHE_FLASH_ATTR onewire_skip(uint8_t pin)
{
    onewire_write(pin, ONEWIRE_SKIP_ROM,0); // Skip ROM
}







//
// Read a byte
//
uint8_t ICACHE_FLASH_ATTR onewire_read(uint8_t pin)
{
	uint8_t bitMask;
	uint8_t r = 0;
	for (bitMask = 0x01; bitMask; bitMask <<= 1) {
		if ( onewire_read_bit(pin)) r |= bitMask;
	}
	return r;
}


//
// Compute a Dallas Semiconductor 8 bit CRC directly.
// this is much slower, but much smaller, than the lookup table.
//
uint8_t ICACHE_FLASH_ATTR onewire_crc8(const uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0;
	uint8_t i;
	while (len--) {
		uint8_t inbyte = *addr++;
		for (i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}

//
// Compute a Dallas Semiconductor 16 bit CRC. I have never seen one of
// these, but here it is.
//
uint16_t ICACHE_FLASH_ATTR onewire_crc16(const uint16_t *data, const uint16_t  len)
{
	uint16_t  i;
	uint16_t  crc = 0;
    for ( i = 0; i < len; i++) {
    	uint16_t cdata = data[len];
    	cdata = (cdata ^ (crc & 0xff)) & 0xff;
    	crc >>= 8;
    	if (oddparity[cdata & 0xf] ^ oddparity[cdata >> 4])
    		crc ^= 0xc001;
    	cdata <<= 6;
    	crc ^= cdata;
    	cdata <<= 1;
    	crc ^= cdata;
    }
    return crc;
}


