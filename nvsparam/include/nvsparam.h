#ifndef __NVSPARAM_H__
#define __NVSPARAM_H__


/**
  * @brief  save param to flash with protect.
  *
  * @param  space_name Namespace name. Maximal length is determined by the
  *                    underlying implementation, but is guaranteed to be
  *                    at least 15 characters. Shouldn't be empty.
  * @param  key  Key name of param. Different params should have different keys.
  *              Maximal length is 15 characters. Shouldn't be empty.
  * @param  param pointer of param.
  * @param  len length of param, Maximum length is 1984 bytes
  *
  * @return
  *     - ESP_OK: succeed
  *     - others: refer to nvs.h
  */
esp_err_t nvs_param_save(const char* space_name, const char* key, void* param, uint16_t len);
esp_err_t nvs_param_i8_save(const char* space_name, const char* key, int8_t param);
esp_err_t nvs_param_u8_save(const char* space_name, const char* key, uint8_t param);
esp_err_t nvs_param_i16_save(const char* space_name, const char* key, int16_t param);
esp_err_t nvs_param_u16_save(const char* space_name, const char* key, uint16_t param);
esp_err_t nvs_param_i32_save(const char* space_name, const char* key, int32_t param);
esp_err_t nvs_param_u32_save(const char* space_name, const char* key, uint32_t param);
esp_err_t nvs_param_i64_save(const char* space_name, const char* key, int64_t param);
esp_err_t nvs_param_u64_save(const char* space_name, const char* key, uint64_t param);
esp_err_t nvs_param_str_save(const char* space_name, const char* key, const char* param);

/**
  * @brief  read param from flash.
  *
  * @param  space_name Namespace name. Maximal length is determined by the
  *                    underlying implementation, but is guaranteed to be
  *                    at least 15 characters. Shouldn't be empty.
  * @param  key  Key name of param. Different params should have different keys.
  *              Maximal length is 15 characters. Shouldn't be empty.
  * @param  dest the address to save read param.
  *
  * @return
  *     - ESP_OK: succeed
  *     - others: refer to nvs.h
  */
esp_err_t nvs_param_load(const char* space_name, const char* key, void* dest);
esp_err_t nvs_param_i8_load(const char* space_name, const char* key, int8_t* dest);
esp_err_t nvs_param_u8_load(const char* space_name, const char* key, uint8_t* dest);
esp_err_t nvs_param_u8_load_def(const char* space_name, const char* key, uint8_t* dest, uint8_t def);
esp_err_t nvs_param_i16_load(const char* space_name, const char* key, int16_t* dest);
esp_err_t nvs_param_u16_load(const char* space_name, const char* key, uint16_t* dest);
esp_err_t nvs_param_i32_load(const char* space_name, const char* key, int32_t* dest);
esp_err_t nvs_param_u32_load(const char* space_name, const char* key, uint32_t* dest);
esp_err_t nvs_param_i64_load(const char* space_name, const char* key, int64_t* dest);
esp_err_t nvs_param_u64_load(const char* space_name, const char* key, uint64_t* dest);
esp_err_t nvs_param_str_load(const char* space_name, const char* key, char* dest);


/**
  * @brief  erase param saved in flash.
  *
  * @param  space_name Namespace name. Maximal length is determined by the
  *                    underlying implementation, but is guaranteed to be
  *                    at least 15 characters. Shouldn't be empty.
  * @param  key  Key name of param. Different params should have different keys.
  *              Maximal length is 15 characters. Shouldn't be empty.
  *
  * @return
  *     - ESP_OK: succeed
  *     - others: refer to nvs.h
  */
esp_err_t nvs_param_erase(const char* space_name, const char* key);


#endif
