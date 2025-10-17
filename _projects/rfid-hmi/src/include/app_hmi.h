/**
 * @file app_hmi.h
 * @brief Nextion HMI interface header.
 * @details Provides UART communication functions to control the Nextion display.
 */

#ifndef APP_HMI_H_
#define APP_HMI_H_

#include <stdint.h>

/**
 * @brief Initializes the UART connection with the Nextion HMI.
 */
void app_hmi_init(void);

/**
 * @brief FreeRTOS task for handling HMI background processes.
 * @param pvParameters Unused task parameter.
 */
void app_hmi_task(void *pvParameters);

/**
 * @brief Displays an access message on the HMI.
 * @param msg Text message to show (e.g., "Access Granted").
 * @param status Status flag (1 = granted, 0 = denied).
 */
void app_hmi_show_message(const char *msg, uint8_t status);

/**
 * @brief Displays access message and UID on the HMI.
 * @param uid UID string to display.
 * @param msg Access message ("Access Granted" / "Access Denied").
 * @param status 1 = granted, 0 = denied.
 */
void app_hmi_show_access(const char *uid, const char *msg, uint8_t status);

#endif /* APP_HMI_H_ */