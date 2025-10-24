
/**
 * @file mfrc522.c
 * @brief MFRC522 RFID driver using ESP-IDF SPI.
 * @details Implements initialization and card reading using SPI polling.
 * @author Hammurabi Sierra
 * @date 2025/10/23
 */


#include "mfrc522.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>

#define PIN_MISO 19
#define PIN_MOSI 23
#define PIN_SCK  18
#define PIN_SS   5
#define PIN_RST  22


static const char *TAG = "MFRC522";
static spi_device_handle_t spi;


static void mfrc522_write(uint8_t reg, uint8_t val) {
    uint8_t data[2] = {(reg << 1) & 0x7E, val};
    spi_transaction_t t = {.length = 16, .tx_buffer = data};
    spi_device_polling_transmit(spi, &t);
}

static uint8_t mfrc522_read(uint8_t reg) {
    uint8_t tx[2] = {((reg << 1) & 0x7E) | 0x80, 0}, rx[2];
    spi_transaction_t t = {.length = 16, .tx_buffer = tx, .rx_buffer = rx};
    spi_device_polling_transmit(spi, &t);
    return rx[1];
}

static void mfrc522_set_bitmask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = mfrc522_read(reg);
    mfrc522_write(reg, tmp | mask);
}

static void mfrc522_clear_bitmask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = mfrc522_read(reg);
    mfrc522_write(reg, tmp & (~mask));
}

static void mfrc522_antenna_on() {
    uint8_t val = mfrc522_read(0x14);
    if (!(val & 0x03)) mfrc522_write(0x14, val | 0x03);
}

static bool mfrc522_transceive(uint8_t *send, uint8_t sendLen, uint8_t *back, uint8_t *backLen) {
    mfrc522_write(0x01, 0x00);
    mfrc522_write(0x02, 0x77 | 0x80);
    mfrc522_clear_bitmask(0x04, 0x80);
    mfrc522_set_bitmask(0x0A, 0x80);
    for (int i = 0; i < sendLen; i++) mfrc522_write(0x09, send[i]);
    mfrc522_write(0x01, 0x0C);
    mfrc522_set_bitmask(0x0D, 0x80);
    int i = 3000;
    uint8_t n;
    do { n = mfrc522_read(0x04); i--; } while (i && !(n & 0x30));
    mfrc522_clear_bitmask(0x0D, 0x80);
    if (i == 0) return false;
    if (mfrc522_read(0x0C) & 0x1F) {
        uint8_t fifoLevel = mfrc522_read(0x0A);
        fifoLevel = (fifoLevel > *backLen) ? *backLen : fifoLevel;
        for (uint8_t j = 0; j < fifoLevel; j++) back[j] = mfrc522_read(0x09);
        *backLen = fifoLevel;
        return true;
    }
    return false;
}

static bool mfrc522_request(uint8_t *atqa) {
    uint8_t req[1] = {0x26};
    uint8_t len = 2;
    mfrc522_write(0x0D, 0x07);
    bool ok = mfrc522_transceive(req, 1, atqa, &len);
    mfrc522_write(0x0D, 0x00);
    return ok;
}

static bool mfrc522_anticoll(mfrc522_uid_t *uid) {
    uint8_t cmd[2] = {0x93, 0x20};
    uint8_t back[10];
    uint8_t backLen = sizeof(back);
    mfrc522_write(0x0D, 0x00);
    if (!mfrc522_transceive(cmd, 2, back, &backLen) || backLen < 5) return false;
    memcpy(uid->bytes, back, 4);
    return true;
}

esp_err_t mfrc522_init(void) {
    ESP_LOGI(TAG, "Initializing MFRC522...");
    spi_bus_config_t buscfg = {.miso_io_num = PIN_MISO, .mosi_io_num = PIN_MOSI, .sclk_io_num = PIN_SCK, .quadwp_io_num = -1, .quadhd_io_num = -1};
    spi_device_interface_config_t devcfg = {.clock_speed_hz = 1000000, .mode = 0, .spics_io_num = PIN_SS, .queue_size = 1};
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    gpio_set_direction(PIN_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    mfrc522_write(0x01, 0x00);
    mfrc522_write(0x2A, 0x8D);
    mfrc522_write(0x2B, 0x3E);
    mfrc522_write(0x2D, 30);
    mfrc522_write(0x2C, 0);
    mfrc522_write(0x15, 0x40);
    mfrc522_write(0x11, 0x3D);
    mfrc522_antenna_on();
    uint8_t version = mfrc522_read(0x37);
    ESP_LOGI(TAG, "MFRC522 version: 0x%02X", version);
    return ESP_OK;
}

bool mfrc522_read_card(mfrc522_uid_t *uid) {
    uint8_t atqa[2];
    if (!mfrc522_request(atqa)) return false;
    if (!mfrc522_anticoll(uid)) return false;
    return true;
}