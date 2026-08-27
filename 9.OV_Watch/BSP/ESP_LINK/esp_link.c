#include "esp_link.h"
#include "usart.h"
#include "ui.h"
#include <string.h>

#define ESP_LINK_LINE_SIZE    96U
#define ESP_LINK_GAP_MS       300U   /* 字节间超时：超过即丢弃残行 */

static uint8_t esp_rx_byte;
static volatile char esp_line[ESP_LINK_LINE_SIZE];
static volatile uint8_t esp_line_length = 0U;
static volatile uint8_t esp_line_ready = 0U;
static volatile uint32_t esp_rx_last_tick = 0U;

void ESP_Link_Init(void)
{
    esp_rx_byte = 0U;
    esp_line_length = 0U;
    esp_line_ready = 0U;
    esp_rx_last_tick = HAL_GetTick();
    HAL_UART_Receive_IT(&huart2, &esp_rx_byte, 1U);
}

/* 接收完成回调：逐字节组行。
 * 注意：HAL 在 ORE/FE/NE 等错误后会关闭 RXNE 中断并停掉接收链，
 * 必须在 HAL_UART_ErrorCallback 里重新挂载，否则串口一次错误后永久静默。 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance != USART2) return;

    esp_rx_last_tick = HAL_GetTick();

    if(!esp_line_ready) {
        if((esp_rx_byte == '\n') || (esp_rx_byte == '\r')) {
            if(esp_line_length > 0U) {
                esp_line[esp_line_length] = '\0';
                esp_line_ready = 1U;
            }
        } else if(esp_line_length < (ESP_LINK_LINE_SIZE - 1U)) {
            esp_line[esp_line_length++] = (char)esp_rx_byte;
        } else {
            esp_line_length = 0U;   /* 超长行直接丢弃，防止黏住后续消息 */
        }
    }

    HAL_UART_Receive_IT(&huart2, &esp_rx_byte, 1U);
}

/* 通信错误（溢出/帧/噪声等）：清空半行状态并重启接收。
 * 发生场景：ESP32 上电瞬间的乱码/波特率抖动、杜邦线接触不良。 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance != USART2) return;

    esp_line_length = 0U;
    esp_line_ready = 0U;

    if(huart->RxState == HAL_UART_STATE_READY) {
        HAL_UART_Receive_IT(&huart2, &esp_rx_byte, 1U);
    }
}

/* 从 *cursor 起解析一个十进制无符号数（0~255），
 * 至少一位数字，成功后 *cursor 停在数字结束位置，返回1；否则返回0 */
static int esp_link_parse_number(const char **cursor, unsigned int *value)
{
    const char *p = *cursor;
    unsigned int acc = 0U;

    if((*p < '0') || (*p > '9')) return 0;

    while((*p >= '0') && (*p <= '9')) {
        acc = (acc * 10U) + (unsigned int)(*p - '0');
        if(acc > 255U) return 0;
        p++;
    }
    *value = acc;
    *cursor = p;
    return 1;
}

static void esp_link_process_time(const char *line)
{
    const char *p = line + 5;           /* 跳过 "TIME," */
    unsigned int hour = 0U;
    unsigned int minute = 0U;
    unsigned int second = 0U;

    if(!esp_link_parse_number(&p, &hour))      { ESP_Link_SendLine("ERR"); return; }
    if(*p != ',')                              { ESP_Link_SendLine("ERR"); return; }
    p++;
    if(!esp_link_parse_number(&p, &minute))    { ESP_Link_SendLine("ERR"); return; }
    if(*p != ',')                              { ESP_Link_SendLine("ERR"); return; }
    p++;
    if(!esp_link_parse_number(&p, &second))    { ESP_Link_SendLine("ERR"); return; }
    /* 行尾只允许空格（\r 已在接收侧剥离） */
    while(*p == ' ') p++;
    if(*p != '\0')                             { ESP_Link_SendLine("ERR"); return; }

    if((hour >= 24U) || (minute >= 60U) || (second >= 60U)) {
        ESP_Link_SendLine("ERR");
        return;
    }

    ui_set_time_from_network((uint8_t)hour, (uint8_t)minute, (uint8_t)second);
    ESP_Link_SendLine("TIME_OK");
}

void ESP_Link_Process(void)
{
    char line[ESP_LINK_LINE_SIZE];
    uint32_t now = HAL_GetTick();

    /* 残行超时清理：ESP32 异常断电/掉线时，半行数据不能黏住下一条消息 */
    if(!esp_line_ready && (esp_line_length > 0U) &&
       ((now - esp_rx_last_tick) > ESP_LINK_GAP_MS)) {
        __disable_irq();
        if(!esp_line_ready) esp_line_length = 0U;
        __enable_irq();
    }

    if(!esp_line_ready) return;
    __disable_irq();
    memcpy((void *)line, (const void *)esp_line, ESP_LINK_LINE_SIZE);
    esp_line_length = 0U;
    esp_line_ready = 0U;
    __enable_irq();

    if(strncmp(line, "TIME,", 5U) == 0) {
        esp_link_process_time(line);
    } else if(strncmp(line, "PING", 5U) == 0) {   /* 严格等于 PING(4字节+\0) */
        ESP_Link_SendLine("PONG");
    } else {
        ESP_Link_SendLine("ERR");
    }
}

void ESP_Link_SendLine(const char *line)
{
    if(line == NULL) return;
    /* 注：本板 H1 无 USART TX 引脚（PA2 未引出），回执物理不可达；代码保留备用 */
    HAL_UART_Transmit(&huart2, (const uint8_t *)line,
                      (uint16_t)strlen(line), 100U);
    HAL_UART_Transmit(&huart2, (const uint8_t *)"\n", 1U, 20U);
}
