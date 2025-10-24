/**
 * @file app_access.c
 * @brief Implementation of access control validation logic.
 * @details Compares scanned RFID UIDs with stored whitelist entries
 *          and updates HMI with access status.
 * @author Hammurabi Sierra
 * @date 2025/10/20
 */

#include "app_access.h"
#include "app_hmi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "ACCESS";


/** @brief User info structure */
typedef struct {
    uint8_t uid[4];
    const char *name;
    const char *member_id;
    int photo_id;
    bool access_granted;
} user_info_t;


/** @brief Whitelist with user details */
static const user_info_t whitelist[] = {
    {{0xC4, 0x52, 0x5E, 0x92}, "John Smith", "Member #0054210", 0, true},
    {{0xA3, 0x31, 0x6B, 0x9A}, "Mary Johnson", "Member #0016423", 1, false}
};

static const int whitelist_size = sizeof(whitelist) / sizeof(whitelist[0]);

void app_access_init(void)
{
    ESP_LOGI(TAG, "Access control initialized");
}

void app_access_verify_uid(uint8_t *uid) {
    char uid_str[16];
    snprintf(uid_str, sizeof(uid_str), "%02X%02X%02X%02X",
             uid[0], uid[1], uid[2], uid[3]);

    for (int i = 0; i < whitelist_size; i++) {
        if (memcmp(uid, whitelist[i].uid, 4) == 0) {
            ESP_LOGI(TAG, "Tag detected (UID: %s)", uid_str);

            if (whitelist[i].access_granted) {
                app_hmi_show_access(uid_str, "Access Granted", 1);
                vTaskDelay(pdMS_TO_TICKS(1500));
                app_hmi_show_user_details(whitelist[i].name, whitelist[i].member_id, whitelist[i].photo_id, whitelist[i].access_granted);
            } else {
                app_hmi_show_access(uid_str, "Access Denied", 0);
                vTaskDelay(pdMS_TO_TICKS(1500));
                app_hmi_show_user_details(whitelist[i].name, whitelist[i].member_id, whitelist[i].photo_id, whitelist[i].access_granted);
            }

            vTaskDelay(pdMS_TO_TICKS(2500)); // Wait before returning to home
            app_hmi_go_home();
            return;
        }
    }

    ESP_LOGI(TAG, "Access Denied (UID: %s)", uid_str);
    app_hmi_show_access(uid_str, "Access Denied", 0);
    vTaskDelay(pdMS_TO_TICKS(2500));
    app_hmi_go_home();
}