/**
 * @file main.c
 * @brief Main application entry point for RFID-HMI Access Control System.
 * @details Initializes the RFID reader, HMI interface, and access control logic.
 *          Spawns FreeRTOS tasks for RFID scanning and HMI updates.
 *
 * @author Hammurabi Sierra
 * @date 2025/10/15
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_rfid.h"
#include "app_hmi.h"
#include "app_access.h"

/**
 * @brief Main entry point of the firmware.
 */
void app_main(void)
{
    printf("Starting Access Control System...\n");

    app_hmi_init();
    app_rfid_init();
    app_access_init();

    xTaskCreate(app_rfid_task, "rfid_task", 4096, NULL, 5, NULL);
    xTaskCreate(app_hmi_task, "hmi_task", 4096, NULL, 5, NULL);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}