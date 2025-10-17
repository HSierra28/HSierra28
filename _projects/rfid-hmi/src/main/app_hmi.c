/**
 * @file app_hmi.c
 * @brief Implementation of Nextion HMI communication.
 * @details Sends formatted UART commands to update the HMI screen with messages
 *          and visual indicators (colors).
 */

#include "app_hmi.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

#define UART_PORT UART_NUM_2
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define BUF_SIZE 1024

static const char *TAG = "HMI";

void app_hmi_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_PORT, BUF_SIZE, BUF_SIZE, 0, NULL, 0);
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI(TAG, "HMI UART initialized");
}
void app_hmi_task(void *pvParameters)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_hmi_show_message(const char *msg, uint8_t status)
{
    char command[64];
    snprintf(command, sizeof(command), "t0.txt=\"%s\"", msg);
    uart_write_bytes(UART_PORT, command, strlen(command));
    const uint8_t end_cmd[3] = {0xFF, 0xFF, 0xFF};
    uart_write_bytes(UART_PORT, (const char *)end_cmd, 3);

    snprintf(command, sizeof(command),
             "t0.pco=%d", status ? 2016 : 63488);  // Verde o rojo
    uart_write_bytes(UART_PORT, command, strlen(command));
    uart_write_bytes(UART_PORT, (const char *)end_cmd, 3);
}

void app_hmi_show_access(const char *uid, const char *msg, uint8_t status)
{
    char command [64];
    const uint8_t end_cmd[3] = {0xFF, 0xFF, 0xFF};

    snprintf(command, sizeof(command), "t0.txt=\"%s\"", msg);
    uart_write_bytes(UART_PORT, command, strlen(command));
    uart_write_bytes(UART_PORT, (const char *)end_cmd, 3);

    snprintf(command, sizeof(command),
             "t0.pco=%d", status ? 2016 : 63488);  // Verde o rojo
    uart_write_bytes(UART_PORT, command, strlen(command));
    uart_write_bytes(UART_PORT, (const char *)end_cmd, 3);
}