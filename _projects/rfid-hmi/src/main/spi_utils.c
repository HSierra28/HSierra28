/**
 * @file spi_utils.c
 * @brief Implementation of SPI helper functions for ESP-IDF.
 */

#include "spi_utils.h"
#include "esp_log.h"

static const char *TAG = "SPI_UTILS";

esp_err_t spi_utils_init(const spi_device_cfg_t *cfg, spi_device_handle_t *handle)
{
    spi_bus_config_t buscfg = {
        .miso_io_num = cfg->miso_io,
        .mosi_io_num = cfg->mosi_io,
        .sclk_io_num = cfg->sck_io,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = cfg->clock_speed_hz,
        .mode = 0, // SPI mode 0
        .spics_io_num = cfg->cs_io,
        .queue_size = 1,
    };

    esp_err_t ret = spi_bus_initialize(cfg->host, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = spi_bus_add_device(cfg->host, &devcfg, handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "SPI bus initialized successfully");
    }

    return ret;
}

esp_err_t spi_utils_transfer(spi_device_handle_t handle, const uint8_t *tx_data, size_t tx_len, uint8_t *rx_data, size_t rx_len)
{
    spi_transaction_t t = {
        .length = tx_len * 8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    esp_err_t ret = spi_device_transmit(handle, &t);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI transmit failed: %s", esp_err_to_name(ret));
    }

    return ret;
}