/**
 * @file app_access.c
 * @brief Implementation of access control validation logic.
 * @details Compares scanned RFID UIDs with stored whitelist entries
 *          and updates HMI with access status.
 */

#include "app_access.h"
#include "app_hmi.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "ACCESS";

/** @brief Predefined list of valid UIDs */
static const uint8_t whitelist[][4] = {
    {0xDE, 0xAD, 0xBE, 0xEF},
    {0x12, 0x34, 0x56, 0x78}
};

static const int whitelist_size = 2;

void app_access_init(void)
{
    ESP_LOGI(TAG, "Access control initialized");
}

void app_access_verify_uid(uint8_t *uid)
{
    char uid_str[16];
    snprintf(uid_str,sizeof(uid_str), "%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);

    for (int i = 0; i < whitelist_size; i++)
    {
        if (memcmp(uid, whitelist[i], 4) == 0)
        {
            ESP_LOGI(TAG, "Access Granted");
            app_hmi_show_message("Access Granted", 1);
            return;
        }
    }
    ESP_LOGI(TAG, "Access Denied");
    app_hmi_show_message("Access Denied", 0);
}