/**
 * @file app_rfid.c
 * @brief Implementation of RFID scanning and tag detection.
 * @details This module initializes and communicates with the MFRC522 reader
 *          to detect RFID tags and pass their UIDs to the access control logic.
 */

#include "app_rfid.h"
#include "app_access.h"
#include "mfrc522.h"
#include "esp_log.h"

static const char *TAG = "RFID";

void app_rfid_init(void)
{
    ESP_LOGI(TAG, "Initializing RFID reader...");
    mfrc522_init();
}

void app_rfid_task(void *pvParameters)
{
    mfrc522_uid_t uid;
    while (1)
    {
        if (mfrc522_read_card(&uid))
        {
            ESP_LOGI(TAG, "Card detected: %02X %02X %02X %02X",
                     uid.bytes[0], uid.bytes[1], uid.bytes[2], uid.bytes[3]);
            app_access_verify_uid(uid.bytes);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}