/**
 * @file spi_utils.h
 * @brief SPI utility functions for ESP-IDF integration.
 * @details Provides initialization and read/write helper functions for SPI devices
 *          such as the MFRC522 RFID reader.
 */

#ifndef SPI_UTILS_H_
#define SPI_UTILS_H_

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

/**
 * @brief SPI bus configuration parameters.
 */
typedef struct
{
    spi_host_device_t host;  /**< SPI host device (e.g. HSPI_HOST, VSPI_HOST). */
    gpio_num_t mosi_io;      /**< GPIO number for MOSI. */
    gpio_num_t miso_io;      /**< GPIO number for MISO. */
    gpio_num_t sck_io;       /**< GPIO number for SCK. */
    gpio_num_t cs_io;        /**< GPIO number for Chip Select (CS). */
    int clock_speed_hz;      /**< SPI clock speed in Hz. */
} spi_device_cfg_t;

/**
 * @brief Initializes the SPI bus and device for MFRC522.
 * @param cfg Pointer to the SPI configuration structure.
 * @param handle Pointer to store the SPI device handle.
 * @return esp_err_t ESP_OK on success, error code otherwise.
 */
esp_err_t spi_utils_init(const spi_device_cfg_t *cfg, spi_device_handle_t *handle);

/**
 * @brief Writes and reads data through SPI.
 * @param handle SPI device handle.
 * @param tx_data Data to transmit.
 * @param tx_len Length of data to transmit.
 * @param rx_data Buffer to store received data.
 * @param rx_len Length of data to receive.
 * @return esp_err_t ESP_OK on success.
 */
esp_err_t spi_utils_transfer(spi_device_handle_t handle, const uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len);

#endif /* SPI_UTILS_H_ */