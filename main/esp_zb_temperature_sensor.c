/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 *
 * Zigbee HA_temperature_sensor Example
 *
 * This example code is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */
#include "esp_zb_temperature_sensor.h"
#include "aht_sensor_driver.h"

#include "esp_check.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ha/esp_zigbee_ha_standard.h"

#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE in idf.py menuconfig to compile sensor (End Device) source code.
#endif

static const char *TAG = "ESP_ZB_TEMP_SENSOR";

static int16_t zb_temperature_to_s16(float temp)
{
    return (int16_t)(temp * 100);
}

static void esp_app_temp_sensor_handler(float temperature, float humidity)
{
    int16_t temperatue_value = zb_temperature_to_s16(temperature);
    int16_t humidity_value = zb_temperature_to_s16(humidity);
    /* Update temperature sensor measured value */
    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_set_attribute_val(HA_ESP_SENSOR_ENDPOINT,
        ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, &temperatue_value, false);
    esp_zb_zcl_set_attribute_val(HA_ESP_SENSOR_ENDPOINT,
        ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID, &humidity_value, false);
    esp_zb_lock_release();
}

esp_err_t temp_deferred_driver_init(void)
{
    ESP_RETURN_ON_ERROR(aht_sensor_driver_init(ESP_TEMP_SENSOR_UPDATE_INTERVAL, esp_app_temp_sensor_handler), TAG,
                        "Failed to initialize temperature sensor");
    return ESP_OK;
}

esp_zb_temperature_meas_cluster_cfg_t esp_zb_temperature_measurement_cluster_config()
{
    esp_zb_temperature_meas_cluster_cfg_t temperature_config;
    temperature_config.measured_value = zb_temperature_to_s16(0);

    return temperature_config;

}

esp_zb_humidity_meas_cluster_cfg_t esp_zb_humidity_measurement_cluster_config()
{
    esp_zb_humidity_meas_cluster_cfg_t humidity_config;
    humidity_config.measured_value = zb_temperature_to_s16(0);

    return humidity_config;

}

esp_zb_zcl_reporting_info_t get_reporting_info() {
    /* Config the reporting info  */
    esp_zb_zcl_reporting_info_t reporting_info = {
        .direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV,
        .ep = HA_ESP_SENSOR_ENDPOINT,
        .cluster_id = ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
        .cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE,
        .dst.profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .u.send_info.min_interval = 1,
        .u.send_info.max_interval = 0,
        .u.send_info.def_min_interval = 1,
        .u.send_info.def_max_interval = 0,
        .u.send_info.delta.u16 = 100,
        .attr_id = ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
        .manuf_code = ESP_ZB_ZCL_ATTR_NON_MANUFACTURER_SPECIFIC,
    };

    return reporting_info;
}
