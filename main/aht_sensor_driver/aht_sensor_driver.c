#include "aht_sensor_driver.h"

#include "esp_err.h"
#include "esp_check.h"
#include "driver/i2c_master.h"
#include "aht20.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_SCL_IO   CONFIG_I2C_MASTER_SCL   /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO   CONFIG_I2C_MASTER_SDA   /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM      I2C_NUM_0               /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ  50000                   /*!< I2C master clock frequency */

static aht20_dev_handle_t aht20 = NULL;
static aht_sensor_callback_t func_ptr;
static uint16_t interval = 1;

static const char *TAG = "ESP_AHT20_SENSOR_DRIVER";

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_bus_init(void) {
    const i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &i2c_conf);

    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "%-20s: %s", "I2C config error: ", esp_err_to_name(ret));
        return ret;
    }

    ret = i2c_driver_install(I2C_MASTER_NUM, i2c_conf.mode, 0, 0, 0);
    return ret;
}

static esp_err_t i2c_sensor_aht20_init(void) {
    aht20_i2c_config_t aht20_conf = {
        .i2c_port = I2C_MASTER_NUM,
        .i2c_addr = AHT20_ADDRRES_0,
    };

    esp_err_t ret = aht20_new_sensor(&aht20_conf, &aht20);
    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "%-20s: %s", "I2C aht20 error: ", esp_err_to_name(ret));
    }
    return ret;
}

/**
 * @brief Tasks for updating the sensor value
 *
 * @param arg      Unused value.
 */
static void aht_sensor_driver_value_update(void *arg) {
    for (;;) {
        uint32_t temperature_raw;
        uint32_t humidity_raw;
        float temperature;
        float humidity;

        // ESP_LOGI(TAG, "%-20s", "getting data for aht20!");

        ESP_ERROR_CHECK(aht20_read_temperature_humidity(aht20, &temperature_raw, &temperature, &humidity_raw, &humidity));
        ESP_LOGI(TAG, "%-20s: %2.2f", "humidity is", humidity);
        ESP_LOGI(TAG, "%-20s: %2.2f degC", "temperature is", temperature);

        if (func_ptr) {
            func_ptr(temperature, humidity);
        }
        vTaskDelay(pdMS_TO_TICKS(interval * 1000));
    }
}

/**
 * @brief init temperature sensor
 *
 * @param config      pointer of temperature sensor config.
 */
static esp_err_t aht_sensor_driver_sensor_init() {
    ESP_ERROR_CHECK(i2c_bus_init());
    ESP_ERROR_CHECK(i2c_sensor_aht20_init());
    if (aht20 == NULL) {
        ESP_LOGI(TAG, "aht20 is null!");
        return ESP_FAIL;
    }
    return (xTaskCreate(aht_sensor_driver_value_update, "aht20_update", 4096 , NULL, 5, NULL) == pdTRUE) ? ESP_OK : ESP_FAIL;
}

esp_err_t aht_sensor_driver_init(uint16_t update_interval,
                             aht_sensor_callback_t cb) {

    esp_err_t ret = aht_sensor_driver_sensor_init();
    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "%-20s: %s", "I2C error: ", esp_err_to_name(ret));
        return ret;
    }
    func_ptr = cb;
    interval = update_interval;
    return ESP_OK;
}
