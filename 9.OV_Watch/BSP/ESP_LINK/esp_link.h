#ifndef __ESP_LINK_H
#define __ESP_LINK_H

/*
 * STM32 <-> ESP32-C3 串口链路（USART2 PA2=TX / PA3=RX，115200 8N1）
 *
 * 行协议（'\n' 结尾，\r 忽略）：
 *   ESP32 -> STM32:
 *     TIME,hour,minute,second   有效范围 0-23/0-59/0-59，成功更新软件时钟并回复 TIME_OK
 *     PING                      链路自检，回复 PONG
 *     其他任意行                回复 ERR
 *   STM32 -> ESP32:
 *     TIME_OK / PONG / ERR（由本模块自动发出，调用方只管收发整行）
 */

/* 发送一行（内部自动补 '\n'），阻塞发送，最长等待 100ms */
void ESP_Link_SendLine(const char *line);

void ESP_Link_Init(void);
void ESP_Link_Process(void);

#endif
