#ifndef __AHT21_H
#define __AHT21_H

#include "stdint.h"

uint8_t AHT21_Init(void);
uint8_t AHT21_Read(int16_t *temperature_x10, uint16_t *humidity_x10);
uint8_t AHT21_GetLatest(int16_t *temperature_x10, uint16_t *humidity_x10);

#endif
