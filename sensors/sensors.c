// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.




#include "sensors.h"


static const char* TAG = "SENS";

uint8_t sensors_group_count = 0;
uint16_t sensors_count = 0;

// static void sensors_recv_queue_cb(void *arg)
// {
//     const portTickType xTicksToWait = 10 / portTICK_RATE_MS;
//     sensor_t sensor;

//     for( ;; )
//     {
//          //xQueueSendToBack(sensors_queue, &sensor, 0);
//         if ( xQueueReceive( sensors_queue, &sensor, xTicksToWait ) == pdPASS )
//         {
//             ESP_LOGW(TAG, "Sens recv: %s value: int = %d, float = %.02f, byte = %d %d %d %d"
//                     , sensor.name
//                     , sensor.value.i32
//                     , sensor.value.f
//                     , sensor.value.b[0]
//                     , sensor.value.b[1]
//                     , sensor.value.b[2]
//                     , sensor.value.b[3]
//                     );
            
//             for ( int16_t i = 0; i < sensors_count; i++)
//             {
//                 if ( strcmp(sensors[i].name, sensor.name) == 0 && sensors[i].id == sensor.id) 
//                 {
//                     memcpy(&sensors[i], &sensor, sizeof(sensor_t));
//                     break;
//                 }
//             }            

//         }           
//     }    
// }

// int8_t sensors_group_add(const char *name)
// {
    
//     for ( uint8_t i = 0; i < sensors_group_count; i++)
//     {
//         if ( strcmp( sensors_groups[i].name, name) == 0 ) {
//             ESP_LOGE(TAG, "%s: group <%s> already exists", __func__, name);
//             return SENSOR_ERROR_ID;
//         }
//     }
    
//     sensors_group_count++;

//     sensors_groups = (sensor_group_t *) realloc(sensors_groups, sensors_group_count * sizeof(sensor_group_t) );
//     sensors_groups[sensors_group_count-1].name = strdup(name);
//     sensors_groups[sensors_group_count-1].id = sensors_group_count-1;

//     return sensors_groups[sensors_group_count-1].id;
// }

int16_t sensors_add(const char *name, func_sensors_print_cb cb, void *args)
{
    for (uint16_t i = 0; i < sensors_count; i++ )
    {
        if ( strcmp(sensors[i].name, name) == 0 
            && sensors[i].fn_cb == cb
        ) 
        {
            ESP_LOGE(TAG, "%s: sensor <%s> already exists", __func__, name);
            return SENSOR_ERROR_ID;            
        }
    }

    sensors_count++;
    sensors = (sensor_t *) realloc(sensors, sensors_count * sizeof(sensor_t));
    //memcpy( &sensors[sensors_count-1], sensor, sizeof(sensor_t));
    //strcpy( sensors[ sensors_count - 1 ].name, name);
    sensors[ sensors_count - 1 ].name = strdup(name);
    sensors[ sensors_count - 1 ].fn_cb = cb;
    sensors[ sensors_count - 1 ].args = args;
    sensors[ sensors_count - 1 ].id = sensors_count-1;

    return sensors[sensors_count-1].id;
}

// sensor_t *sensors_get_by_id(int16_t id)
// {
//     // sensor_t *sensor = calloc(1, sizeof(sensor_t));
//     // sensor->id = -1;

//     for ( int16_t i = 0; i < sensors_count; i++)
//     {
//         if ( sensors[i].id == id ) 
//         {
//             return (sensor_t *)&sensors[i];
//         }
//     }
//     return NULL;
// }

// sensor_t *sensors_get_by_name(const char *name)
// {
//     //sensor_t *sensor = calloc(1, sizeof(sensor_t));
//     //sensor->id = -1;

//     for ( int16_t i = 0; i < sensors_count; i++)
//     {
//         if ( strcmp(sensors[i].name, name) == 0 ) 
//         {
//             return (sensor_t *)&sensors[i];
//         }
//     }
//     return NULL;
// }

// data_val_t sensors_get_value_by_id(int16_t id)
// {
//     data_val_t data_val;
//     data_val.i32 = 0;

//     sensor_t *sensor = (sensor_t *) calloc(1, sizeof(sensor_t));
//     sensor = sensors_get_by_id(id);

//     if ( sensor != NULL )
//         data_val = sensor->value;
//     free(sensor);

//     return data_val;
// }

// data_val_t sensors_get_value_by_name(const char *name)
// {
//     data_val_t data_val;
//     data_val.i32 = 0;

//     sensor_t *sensor = (sensor_t *) calloc(1, sizeof(sensor_t));
//     sensor = sensors_get_by_name(name);

//     if ( sensor != NULL )
//         data_val = sensor->value;
//     free(sensor);

//     return data_val;    
// }

// void sensors_set_value_by_id(uint16_t id, data_val_t value)
// {
//     for ( int16_t i = 0; i < sensors_count; i++)
//     {
//         if ( sensors[i].id == id ) 
//         {
//             memcpy(&sensors[i].value, &value, sizeof(data_val_t));
//             break;
//         }
//     }
// }

// void sensors_set_value_by_name(const char *name, data_val_t value)
// {
//     for ( int16_t i = 0; i < sensors_count; i++)
//     {
//         if ( strcmp(sensors[i].name, name) == 0 ) 
//         {
//             memcpy(&sensors[i].value, &value, sizeof(data_val_t));
//             break;
//         }
//     }
// }

void sensors_init()
{
//     //sensors_queue = xQueueCreate(SENSORS_QUEUE_SIZE, sizeof(sensor_t));
//     //xTaskCreate(sensors_recv_queue_cb, "sens_recv", 1024 + 256, NULL, 10, NULL); // При 1024 иногда случался Stack canary watchpoint triggered
//     //sensors_group_add("generic");
}

