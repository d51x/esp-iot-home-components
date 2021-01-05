#ifndef __BUTTON_H__
#define __BUTTON_H__


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/timers.h"
#include "freertos/queue.h"


typedef void *button_handle_t;      // button object
typedef void (* button_cb)(void *); // указатель на функцию call back ???


typedef enum {
    BUTTON_STATE_IDLE = 0,
    BUTTON_STATE_PUSH,
    BUTTON_STATE_PRESSED,
} button_status_t;

typedef enum {
    BUTTON_ACTIVE_HIGH = 1,    /*!<button active level: high level*/
    BUTTON_ACTIVE_LOW = 0,     /*!<button active level: low level*/
} button_active_t;

typedef enum {
    BUTTON_CB_PUSH = 0,   /*!<button push callback event */          // срабатывает сразу при нажатии, не учитывая удержание и отпускание
    BUTTON_CB_RELEASE,    /*!<button release callback event */       // срабатывает при отпускании, не учитывает удержание
    BUTTON_CB_TAP,        /*!<button quick tap callback event(will not trigger if there already is a "PRESS" event) */ // всегда присутствует перед RELEASE
    BUTTON_CB_SERIAL,     /*!<button serial trigger callback event */  // срабатывает при удержании после 
} button_cb_type_t;

typedef struct button button_t;
typedef struct btn_cb button_cb_t;

struct btn_cb {
    TickType_t interval;
    button_cb cb;
    void *arg;
    uint8_t on_press;
    TimerHandle_t tmr;
    button_t *pbtn;
    button_cb_t *next_cb;
};

struct button {
    uint8_t io_num;
    uint8_t active_level;
    uint32_t serial_thres_sec;
    uint8_t taskq_on;
    QueueHandle_t taskq;
    QueueHandle_t argq;
    button_status_t state;
    button_cb_t tap_short_cb;
    button_cb_t tap_psh_cb;
    button_cb_t tap_psh2_cb;
    button_cb_t tap_rls_cb;
    button_cb_t press_serial_cb;
    button_cb_t *cb_head;
};
/**
 * @brief Init button functions
 *
 * @param gpio_num GPIO index of the pin that the button uses
 * @param active_level button hardware active level.
 *        For "BUTTON_ACTIVE_LOW" it means when the button pressed, the GPIO will read low level.
 *
 * @return A button_handle_t handle to the created button object, or NULL in case of error.
 */
button_handle_t button_create(gpio_num_t gpio_num, button_active_t active_level);


/**
 * @brief Register a callback function for a serial trigger event.
 *
 * @param btn_handle handle of the button object
 * @param start_after_sec define the time after which to start serial trigger action
 * @param interval_tick serial trigger interval
 * @param cb callback function for "TAP" action.
 * @param arg Parameter for callback function
 * @note
 *        Button callback functions execute in the context of the timer service task.
 *        It is therefore essential that button callback functions never attempt to block.
 *        For example, a button callback function must not call vTaskDelay(), vTaskDelayUntil(),
 *        or specify a non zero block time when accessing a queue or a semaphore.
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Parameter error
 */
esp_err_t button_set_serial_cb(button_handle_t btn_handle, uint32_t start_after_sec, TickType_t interval_tick, button_cb cb, void *arg);

/**
 * @brief Register a callback function for a button_cb_type_t action.
 *
 * @param btn_handle handle of the button object
 * @param type callback function type
 * @param cb callback function for "TAP" action.
 * @param arg Parameter for callback function
 * @note
 *        Button callback functions execute in the context of the timer service task.
 *        It is therefore essential that button callback functions never attempt to block.
 *        For example, a button callback function must not call vTaskDelay(), vTaskDelayUntil(),
 *        or specify a non zero block time when accessing a queue or a semaphore.
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Parameter error
 */
esp_err_t button_set_evt_cb(button_handle_t btn_handle, button_cb_type_t type, button_cb cb, void *arg);

/**
 * @brief Callbacks invoked as timer events occur while button is pressed.
 *        Example: If a button is configured for 2 sec, 5 sec and 7 sec callbacks and 
 *                 if the button is pressed for 6 sec then 2 sec callback would be invoked at 2 sec event and 
 *                 5 sec callback would be invoked at 5 sec event
 *
 * @param btn_handle handle of the button object
 * @param press_sec the callback function would be called if you press the button for a specified period of time
 * @param cb callback function for "PRESS and HOLD" action.
 * @param arg Parameter for callback function
 *
 * @note
 *        Button callback functions execute in the context of the timer service task.
 *        It is therefore essential that button callback functions never attempt to block.
 *        For example, a button callback function must not call vTaskDelay(), vTaskDelayUntil(),
 *        or specify a non zero block time when accessing a queue or a semaphore.
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Parameter error
 */
esp_err_t button_add_on_press_cb(button_handle_t btn_handle, uint32_t press_sec, button_cb cb, void *arg);

/**
 * @brief Single callback invoked according to the latest timer event on button release.
 *        Example: If a button is configured for 2 sec, 5 sec and 7 sec callbacks and if the button is released at 6 sec then only 5 sec callback would be invoked
 *
 * @param btn_handle handle of the button object
 * @param press_sec the callback function would be called if you press the button for a specified period of time
 * @param cb callback function for "PRESS and RELEASE" action.
 * @param arg Parameter for callback function
 *
 * @note
 *        Button callback functions execute in the context of the timer service task.
 *        It is therefore essential that button callback functions never attempt to block.
 *        For example, a button callback function must not call vTaskDelay(), vTaskDelayUntil(),
 *        or specify a non zero block time when accessing a queue or a semaphore.
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Parameter error
 */
esp_err_t button_add_on_release_cb(button_handle_t btn_handle, uint32_t press_sec, button_cb cb, void *arg);

/**
 * @brief Delete button object and free memory
 * @param btn_handle handle of the button object
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_FAIL Parameter error
 */
esp_err_t button_delete(button_handle_t btn_handle);

/**
 * @brief Remove callback
 *
 * @param btn_handle The handle of the button object
 * @param type callback function event type
 *
 * @return
 *     - ESP_OK Success
 */
esp_err_t button_rm_cb(button_handle_t btn_handle, button_cb_type_t type);

//void configure_push_button(int gpio_num, button_active_t level, void (*btn_cb)(void *), void (*btn_press_cb)(void *));
button_handle_t configure_push_button(int gpio_num, button_active_t level);

esp_err_t button_set_on_presscount_cb(button_handle_t btn_handle, uint32_t pressed_interval, uint8_t cbs_count, button_cb *cb, void *args);


/* TODO:
 *  /buttons
 *  добавить/удалить/изменить настройки кнопки (nvs)
 *  gpio для кнопки
 *  единичное нажатие - функция
 *  двойное, тройное
 *  удержание кнопки на время Х
*/
#endif