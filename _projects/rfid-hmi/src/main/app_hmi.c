/**
 * @file app_hmi.c
 * @brief Implementation of Nextion HMI communication.
 * @details Sends formatted UART commands to update the HMI screen with messages
 *          and visual indicators (colors).
 * @author Hammurabi Sierra
 * @date 2025/10/20
 */

#include "app_hmi.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

#define UART_PORT UART_NUM_2
#define TXD_PIN GPIO_NUM_17
#define RXD_PIN GPIO_NUM_16
#define BUF_SIZE 1024


static const char *TAG = "HMI";

static void send_cmd(const char *cmd){
    uart_write_bytes(UART_PORT, cmd, strlen(cmd));
    const uint8_t end[3] = {0xFF, 0xFF, 0xFF};
    uart_write_bytes(UART_PORT, (const char *)end, 3);
}

void app_hmi_init(void){
    const uart_config_t cfg = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_PORT, BUF_SIZE, BUF_SIZE, 0, NULL, 0);
    uart_param_config(UART_PORT, &cfg);
    uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI(TAG, "HMI UART initialized");

    //Show initial message
    app_hmi_show_message("Waiting for TAG", 0);
}


void app_hmi_task(void *pvParameters){
    while (1){
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_hmi_show_message(const char *msg, uint8_t status) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "t0.txt=\"%s\"", msg);
    send_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "t0.pco=%d", status ? 2016 : 63488);
    send_cmd(cmd);
}


void app_hmi_show_access(const char *uid, const char *msg, uint8_t status) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "t0.txt=\"%s\"", msg);
    send_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "t1.txt=\"UID: %s\"", uid);
    send_cmd(cmd);
    snprintf(cmd, sizeof(cmd), "t0.pco=%d", status ? 2016 : 63488);
    send_cmd(cmd);
}

void app_hmi_show_user_details(const char *name, const char *member_id, int photo_id, bool access_granted) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "page user");
    send_cmd(cmd);

    snprintf(cmd, sizeof(cmd), "name.txt=\"%s\"", name);
    send_cmd(cmd);

    snprintf(cmd, sizeof(cmd), "id.txt=\"%s\"", member_id);
    send_cmd(cmd);

    snprintf(cmd, sizeof(cmd), "pic.pic=%d", photo_id); // Dynamic image ID
    send_cmd(cmd);

    if (access_granted) {
        snprintf(cmd, sizeof(cmd), "status.txt=\"Welcome!\"");
        send_cmd(cmd);
        snprintf(cmd, sizeof(cmd), "status.pco=2016");
        send_cmd(cmd);
    } else {
        snprintf(cmd, sizeof(cmd), "status.txt=\"Access Denied\"");
        send_cmd(cmd);
        snprintf(cmd, sizeof(cmd), "status.pco=63488");
        send_cmd(cmd);
    }
}


void app_hmi_go_home(void){
    send_cmd("page home");
    app_hmi_show_message("Waiting for TAG", 0);
}