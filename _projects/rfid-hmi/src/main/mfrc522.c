/**
 * @file mfrc522.c
 * @brief Implementation of MFRC522 RFID reader driver for ESP-IDF.
 * @details Handles SPI communication with MFRC522 for real UID reading via ISO14443A: Request -> Anticollision -> Select
 */

#include "mfrc522.h"
#include "spi_utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>

#define MFRC522_RST_PIN  GPIO_NUM_22

/* Registers */
#define MFRC522_REG_COMMAND      0x01
#define MFRC522_REG_COMMIEN      0x02
#define MFRC522_REG_COMIRQ       0x04
#define MFRC522_REG_ERROR        0x06
#define MFRC522_REG_STATUS1      0x07
#define MFRC522_REG_STATUS2      0x08
#define MFRC522_REG_FIFO_DATA    0x09
#define MFRC522_REG_FIFO_LEVEL   0x0A
#define MFRC522_REG_CONTROL      0x0C
#define MFRC522_REG_BIT_FRAMING  0x0D
#define MFRC522_REG_COLL         0x0E
#define MFRC522_REG_MODE         0x11
#define MFRC522_REG_TX_MODE      0x12
#define MFRC522_REG_RX_MODE      0x13
#define MFRC522_REG_TX_CONTROL   0x14
#define MFRC522_REG_CRC_RESULT_L 0x22
#define MFRC522_REG_CRC_RESULT_M 0x21
#define MFRC522_REG_T_MODE       0x2A
#define MFRC522_REG_T_PRESCALER  0x2B
#define MFRC522_REG_T_RELOAD_L   0x2D
#define MFRC522_REG_T_RELOAD_H   0x2C
#define MFRC522_REG_RF_CFG       0x26
#define MFRC522_REG_VERSION      0x37

/* Commands */
#define PCD_IDLE        0x00
#define PCD_CALCCRC     0x03
#define PCD_TRANSCEIVE  0x0C
#define PCD_SOFTRESET   0x0F

/* PICC commands */
#define PICC_REQIDL     0x26
#define PICC_ANTICOLL   0x93

static const char *TAG = "MFRC522";
static spi_device_handle_t spi_handle;

/* --- SPI helpers --- */
static void mfrc522_write_register(uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = { (uint8_t)((reg << 1) & 0x7E), val };
    spi_utils_transfer(spi_handle, tx, 2, NULL, 0);
}

static uint8_t mfrc522_read_register(uint8_t reg)
{
    uint8_t tx[2] = { (uint8_t)((reg << 1) | 0x80), 0x00 };
    uint8_t rx[2] = {0};
    spi_utils_transfer(spi_handle, tx, 2, rx, 2);
    return rx[1];
}

static void mfrc522_set_bit_mask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = mfrc522_read_register(reg);
    mfrc522_write_register(reg, tmp | mask);
}

static void mfrc522_clear_bit_mask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = mfrc522_read_register(reg);
    mfrc522_write_register(reg, tmp & (~mask));
}

/* Enable antenna (bit0 & bit1 = 1) */
static void mfrc522_antenna_on(void)
{
    uint8_t val = mfrc522_read_register(MFRC522_REG_TX_CONTROL);
    if ((val & 0x03) == 0)
        mfrc522_set_bit_mask(MFRC522_REG_TX_CONTROL, 0x03);
}

/* --- Core communication --- */
static bool mfrc522_transceive(uint8_t *send_data, uint8_t send_len, uint8_t *back_data, uint8_t *back_len)
{
    uint8_t irqEn = 0x77;
    uint8_t waitIRq = 0x30;  // RxIRq | IdleIRq
    uint8_t n, lastBits;

    mfrc522_write_register(MFRC522_REG_COMMIEN, irqEn | 0x80);
    mfrc522_clear_bit_mask(MFRC522_REG_COMIRQ, 0x80);
    mfrc522_set_bit_mask(MFRC522_REG_FIFO_LEVEL, 0x80);  // flush FIFO

    // write to FIFO
    for (uint8_t i = 0; i < send_len; i++)
        mfrc522_write_register(MFRC522_REG_FIFO_DATA, send_data[i]);

    // start transmission
    mfrc522_write_register(MFRC522_REG_COMMAND, PCD_TRANSCEIVE);
    mfrc522_set_bit_mask(MFRC522_REG_BIT_FRAMING, 0x80);

    // wait for response
    int i = 0;
    for (; i < 1000; i++)
    {
        n = mfrc522_read_register(MFRC522_REG_COMIRQ);
        if (n & waitIRq)
            break;
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    mfrc522_clear_bit_mask(MFRC522_REG_BIT_FRAMING, 0x80);

    if (i == 1000)
    {
        ESP_LOGW(TAG, "Transceive timeout");
        return false;
    }

    uint8_t error = mfrc522_read_register(MFRC522_REG_ERROR);
    if (error & 0x1B)
    {
        ESP_LOGW(TAG, "Transceive error reg: 0x%02X", error);
        return false;
    }

    if (back_data && back_len)
    {
        uint8_t fifoLevel = mfrc522_read_register(MFRC522_REG_FIFO_LEVEL);
        lastBits = mfrc522_read_register(MFRC522_REG_CONTROL) & 0x07;
        if (lastBits)
            *back_len = (fifoLevel - 1) * 8 + lastBits;
        else
            *back_len = fifoLevel * 8;

        if (fifoLevel > 16)
            fifoLevel = 16;

        for (uint8_t i = 0; i < fifoLevel; i++)
            back_data[i] = mfrc522_read_register(MFRC522_REG_FIFO_DATA);
    }

    mfrc522_write_register(MFRC522_REG_COMMAND, PCD_IDLE);
    return true;
}

/* REQA – detect presence */
static bool mfrc522_request(uint8_t reqMode)
{
    uint8_t cmd[1] = { reqMode };
    uint8_t back_data[16];
    uint8_t back_len = sizeof(back_data);

    mfrc522_write_register(MFRC522_REG_BIT_FRAMING, 0x07);
    bool ok = mfrc522_transceive(cmd, 1, back_data, &back_len);
    mfrc522_write_register(MFRC522_REG_BIT_FRAMING, 0x00);
    return ok && back_len >= 16;
}

/* Get UID (anticollision cascade level 1) */
static bool mfrc522_anticoll(mfrc522_uid_t *uid)
{
    uint8_t cmd[2] = { PICC_ANTICOLL, 0x20 };
    uint8_t back_data[10] = {0};
    uint8_t back_len = sizeof(back_data);

    mfrc522_write_register(MFRC522_REG_BIT_FRAMING, 0x00);
    if (!mfrc522_transceive(cmd, 2, back_data, &back_len))
        return false;

    if (back_len < 40)
    {
        ESP_LOGW(TAG, "Anticoll invalid len: %d", back_len);
        return false;
    }

    // extract UID (4 bytes)
    for (int i = 0; i < 4; i++)
        uid->bytes[i] = back_data[i];

    uint8_t bcc = back_data[4];
    uint8_t calc = uid->bytes[0] ^ uid->bytes[1] ^ uid->bytes[2] ^ uid->bytes[3];
    if (bcc != calc)
    {
        ESP_LOGW(TAG, "BCC mismatch (calc=0x%02X, recv=0x%02X)", calc, bcc);
        return false;
    }

    return true;
}

/* --- Public API --- */
esp_err_t mfrc522_init(void)
{
    ESP_LOGI(TAG, "Initializing MFRC522...");

    gpio_reset_pin(MFRC522_RST_PIN);
    gpio_set_direction(MFRC522_RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(MFRC522_RST_PIN, 1);

    spi_device_cfg_t spi_cfg = {
        .host = HSPI_HOST,
        .mosi_io = GPIO_NUM_23,
        .miso_io = GPIO_NUM_19,
        .sck_io = GPIO_NUM_18,
        .cs_io = GPIO_NUM_5,
        .clock_speed_hz = 1000000  // safe start
    };

    esp_err_t ret = spi_utils_init(&spi_cfg, &spi_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    mfrc522_write_register(MFRC522_REG_COMMAND, PCD_SOFTRESET);
    vTaskDelay(pdMS_TO_TICKS(50));

    // recommended init
    mfrc522_write_register(MFRC522_REG_T_MODE, 0x8D);
    mfrc522_write_register(MFRC522_REG_T_PRESCALER, 0x3E);
    mfrc522_write_register(MFRC522_REG_T_RELOAD_L, 30);
    mfrc522_write_register(MFRC522_REG_T_RELOAD_H, 0);
    mfrc522_write_register(MFRC522_REG_MODE, 0x3D);
    mfrc522_write_register(MFRC522_REG_RF_CFG, 0x70);  // max gain

    mfrc522_antenna_on();
    vTaskDelay(pdMS_TO_TICKS(20));

    uint8_t version = mfrc522_read_register(MFRC522_REG_VERSION);
    ESP_LOGI(TAG, "MFRC522 Version: 0x%02X", version);
    if (version == 0x00 || version == 0xFF)
        ESP_LOGW(TAG, "Warning: Invalid version response, check wiring.");

    return ESP_OK;
}

bool mfrc522_read_card(mfrc522_uid_t *uid)
{
    if (!mfrc522_request(PICC_REQIDL))
        return false;

    if (!mfrc522_anticoll(uid))
        return false;

    ESP_LOGI(TAG, "Card UID: %02X %02X %02X %02X",
             uid->bytes[0], uid->bytes[1], uid->bytes[2], uid->bytes[3]);
    return true;
}