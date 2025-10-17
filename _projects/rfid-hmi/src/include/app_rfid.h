/**
 * @file app_rfid.h
 * @brief RFID reader application layer header.
 * @details Handles initialization and reading of RFID tags via MFRC522 module.
 */

#ifndef APP_RFID_H_
#define APP_RFID_H_

#include "stdint.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Initializes the RFID reader hardware and SPI interface.
 */
void app_rfid_init(void);

/**
 * @brief FreeRTOS task that continuously scans for RFID cards.
 * @param pvParameters Unused task parameter.
 */
void app_rfid_task(void *pvParameters);

#endif /* APP_RFID_H_ */