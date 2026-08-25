#include "esp_link.h"
#include "usart.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>

#define ESP_LINK_LINE_SIZE 96U

static uint8_t esp_rx_byte;
static char esp_line[ESP_LINK_LINE_SIZE];
static uint8_t esp_line_length = 0U;
static volatile uint8_t esp_line_ready = 0U;

void ESP_Link_Init(void)
{
    esp_rx_byte = 0U;
    esp_line_length = 0U;
    esp_line_ready = 0U;
    HAL_UART_Receive_IT(&huart6, &esp_rx_byte, 1U);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART6) {
        if(esp_line_ready) {
            HAL_UART_Receive_IT(&huart6, &esp_rx_byte, 1U);
            return;
        }
        if(esp_rx_byte == '\n' || esp_rx_byte == '\r') {
            if(esp_line_length > 0U) {
                esp_line[esp_line_length] = '\0';
                esp_line_ready = 1U;
            }
        } else if(esp_line_length < (ESP_LINK_LINE_SIZE - 1U)) {
            esp_line[esp_line_length++] = (char)esp_rx_byte;
        } else {
            esp_line_length = 0U;
        }
        HAL_UART_Receive_IT(&huart6, &esp_rx_byte, 1U);
    }
}

static void esp_link_process_time(const char *line)
{
    unsigned int hour;
    unsigned int minute;
    unsigned int second;

    if(sscanf(line, "TIME,%u,%u,%u", &hour, &minute, &second) == 3) {
        if(hour < 24U && minute < 60U && second < 60U) {
            ui_set_time_from_network((uint8_t)hour, (uint8_t)minute, (uint8_t)second);
        }
    }
}

void ESP_Link_Process(void)
{
    char line[ESP_LINK_LINE_SIZE];

    if(!esp_line_ready) return;
    __disable_irq();
    strcpy(line, esp_line);
    esp_line_length = 0U;
    esp_line_ready = 0U;
    __enable_irq();

    if(strncmp(line, "TIME,", 5) == 0) esp_link_process_time(line);
}
