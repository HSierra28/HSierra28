/**
 * @file mfrc522.h
 * @brief MFRC522 RFID reader driver for ESP-IDF.
 * @details Implements communication with the MFRC522 module via SPI.
 *          Provides APIs for initialization, card detection, and UID reading.
 */

#ifndef MFRC522_H_
#define MFRC522_H_

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief MFRC522 UID structure (4-byte UID).
 */
typedef struct
{
    uint8_t bytes[4];
} mfrc522_uid_t;

/**
 * @brief Initializes the MFRC522 RFID reader.
 * @return esp_err_t ESP_OK on success, otherwise error.
 */
esp_err_t mfrc522_init(void);

/**
 * @brief Checks for RFID cards and retrieves UID if present.
 * @param uid Pointer to structure to store the UID.
 * @return true if a card is detected, false otherwise.
 */
bool mfrc522_read_card(mfrc522_uid_t *uid);

#endif /* MFRC522_H_ */