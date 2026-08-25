#ifndef __USER_SENSOR_TASK_H
#define __USER_SENSOR_TASK_H

#include "stdint.h"

void SensorTask(void *argument);
uint8_t SensorTask_GetLatest(char *temperature, uint16_t temperature_size,
                             char *humidity, uint16_t humidity_size);

#endif
