#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvsparam.h"

#define PARAM_CHECK(tag, a, ret)  if(!(a)) {                                 \
        ESP_LOGE(tag,"%s:%d (%s)", __FILE__, __LINE__, __FUNCTION__);      \
        goto ret;                                                      \
        }

#define PARAM_ERR_ASSERT(tag, param, ret)  PARAM_CHECK(tag, (param) == ESP_OK, ret)
#define PARAM_POINT_ASSERT(tag, param, ret) PARAM_CHECK(tag, (param) != NULL, ret)

static const char* TAG = "NVSPARAM";

esp_err_t nvs_param_save(const char* space_name, const char* key, void *param, uint16_t len)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    //PARAM_POINT_ASSERT(TAG, param, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_set_blob(my_handle, key, param, len);
    PARAM_ERR_ASSERT(TAG, ret, SAVE_FINISH);
    ret = nvs_commit(my_handle);

SAVE_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_i8_save(const char* space_name, const char* key, int8_t param)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    //PARAM_POINT_ASSERT(TAG, param, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_set_i8(my_handle, key, param);
    PARAM_ERR_ASSERT(TAG, ret, SAVE_FINISH);
    ret = nvs_commit(my_handle);

SAVE_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_u8_save(const char* space_name, const char* key, uint8_t param)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    //PARAM_POINT_ASSERT(TAG, param, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_set_u8(my_handle, key, param);
    PARAM_ERR_ASSERT(TAG, ret, SAVE_FINISH);
    ret = nvs_commit(my_handle);

SAVE_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_i16_save(const char* space_name, const char* key, int16_t param)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    //PARAM_POINT_ASSERT(TAG, param, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_set_i16(my_handle, key, param);
    PARAM_ERR_ASSERT(TAG, ret, SAVE_FINISH);
    ret = nvs_commit(my_handle);

SAVE_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_u16_save(const char* space_name, const char* key, uint16_t param)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    //PARAM_POINT_ASSERT(TAG, param, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_set_u16(my_handle, key, param);
    PARAM_ERR_ASSERT(TAG, ret, SAVE_FINISH);
    ret = nvs_commit(my_handle);

SAVE_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_i32_save(const char* space_name, const char* key, int32_t param)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    //PARAM_POINT_ASSERT(TAG, param, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_set_i32(my_handle, key, param);
    PARAM_ERR_ASSERT(TAG, ret, SAVE_FINISH);
    ret = nvs_commit(my_handle);

SAVE_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_u32_save(const char* space_name, const char* key, uint32_t param)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    //PARAM_POINT_ASSERT(TAG, param, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_set_u32(my_handle, key, param);
    PARAM_ERR_ASSERT(TAG, ret, SAVE_FINISH);
    ret = nvs_commit(my_handle);
SAVE_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_i64_save(const char* space_name, const char* key, int64_t param)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    //PARAM_POINT_ASSERT(TAG, param, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_set_i64(my_handle, key, param);
    PARAM_ERR_ASSERT(TAG, ret, SAVE_FINISH);
    ret = nvs_commit(my_handle);

SAVE_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_u64_save(const char* space_name, const char* key, uint64_t param)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    //PARAM_POINT_ASSERT(TAG, param, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_set_u64(my_handle, key, param);
    PARAM_ERR_ASSERT(TAG, ret, SAVE_FINISH);
    ret = nvs_commit(my_handle);

SAVE_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_str_save(const char* space_name, const char* key, const char* param)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    //PARAM_POINT_ASSERT(TAG, param, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_set_str(my_handle, key, param);
    PARAM_ERR_ASSERT(TAG, ret, SAVE_FINISH);
    ret = nvs_commit(my_handle);

SAVE_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_load(const char* space_name, const char* key, void* dest)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, dest, OPEN_FAIL);
    nvs_handle my_handle;
    size_t required_size = 0;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_get_blob(my_handle, key, NULL, &required_size);
    PARAM_ERR_ASSERT(TAG, ret, LOAD_FINISH)
    if (required_size == 0) {
        ESP_LOGW(TAG, "the target you want to load has never been saved");
        ret = ESP_FAIL;
        goto LOAD_FINISH;
    }
    ret = nvs_get_blob(my_handle, key, dest, &required_size);

LOAD_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_i8_load(const char* space_name, const char* key, int8_t* dest)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, dest, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_get_i8(my_handle, key, dest);
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_u8_load(const char* space_name, const char* key, uint8_t* dest)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, dest, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_get_u8(my_handle, key, dest);
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}


esp_err_t nvs_param_i16_load(const char* space_name, const char* key, int16_t* dest)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, dest, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_get_i16(my_handle, key, dest);
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_u16_load(const char* space_name, const char* key, uint16_t* dest)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, dest, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_get_u16(my_handle, key, dest);
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

esp_err_t nvs_param_i32_load(const char* space_name, const char* key, int32_t* dest)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, dest, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_get_i32(my_handle, key, dest);
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}


esp_err_t nvs_param_u32_load(const char* space_name, const char* key, uint32_t* dest)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, dest, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_get_u32(my_handle, key, dest);
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}


esp_err_t nvs_param_i64_load(const char* space_name, const char* key, int64_t* dest)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, dest, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_get_i64(my_handle, key, dest);
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}


esp_err_t nvs_param_u64_load(const char* space_name, const char* key, uint64_t* dest)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, dest, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_get_u64(my_handle, key, dest);
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}


esp_err_t nvs_param_str_load(const char* space_name, const char* key, char* dest)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, dest, OPEN_FAIL);
    nvs_handle my_handle;
    size_t required_size = 0;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_get_str(my_handle, key, NULL, &required_size);
    PARAM_ERR_ASSERT(TAG, ret, LOAD_FINISH)
    if (required_size == 0) {
        ESP_LOGW(TAG, "the target you want to load has never been saved");
        ret = ESP_FAIL;
        goto LOAD_FINISH;
    }
    ret = nvs_get_str(my_handle, key, dest, &required_size);

LOAD_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}


esp_err_t nvs_param_erase(const char* space_name, const char* key)
{
    esp_err_t ret = ESP_ERR_INVALID_ARG;
    PARAM_POINT_ASSERT(TAG, space_name, OPEN_FAIL);
    PARAM_POINT_ASSERT(TAG, key, OPEN_FAIL);
    nvs_handle my_handle;
    ret = nvs_open(space_name, NVS_READWRITE, &my_handle);
    PARAM_ERR_ASSERT(TAG, ret, OPEN_FAIL);
    ret = nvs_erase_key(my_handle, key);
    PARAM_ERR_ASSERT(TAG, ret, ERASE_FINISH)
    ret = nvs_commit(my_handle);

ERASE_FINISH:
    nvs_close(my_handle);

OPEN_FAIL:
    return ret;
}

