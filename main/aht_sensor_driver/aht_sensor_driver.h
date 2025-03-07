#pragma once

#include "driver/temperature_sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_I2C_MASTER_SCL GPIO_NUM_1  /*!< gpio number for I2C master clock */
#define CONFIG_I2C_MASTER_SDA GPIO_NUM_0  /*!< gpio number for I2C master data  */

/** Temperature sensor callback
 *
 * @param[in] temperature temperature value in degrees Celsius and humidity from sensor
 *
 */
typedef void (*aht_sensor_callback_t)(float temperature, float humidity);

/**
 * @brief init function for temp sensor and callback setup
 *
 * @param config                pointer of temperature sensor config.
 * @param update_interval       sensor value update interval in seconds.
 * @param cb                    callback pointer.
 *
 * @return ESP_OK if the driver initialization succeed, otherwise ESP_FAIL.
 */
esp_err_t aht_sensor_driver_init(uint16_t update_interval, aht_sensor_callback_t cb);

#ifdef __cplusplus
} // extern "C"
#endif
