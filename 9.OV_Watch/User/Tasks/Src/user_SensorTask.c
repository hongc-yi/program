#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "aht21.h"
#include "user_SensorTask.h"
#include <stdio.h>
#include <string.h>

static char sensor_temperature[16] = "--.- C";
static char sensor_humidity[16] = "--.- %";
static volatile uint8_t sensor_ready = 0U;

void SensorTask(void *argument)
{
    int16_t temperature_x10;
    uint16_t humidity_x10;
    (void)argument;

    osDelay(500U);
    AHT21_Init();
    for(;;) {
        if(AHT21_Read(&temperature_x10, &humidity_x10) == 0U) {
            taskENTER_CRITICAL();
            snprintf(sensor_temperature, sizeof(sensor_temperature), "%d.%d C", 
                     (int)(temperature_x10 / 10),
                     (int)(temperature_x10 < 0 ? -(temperature_x10 % 10) : temperature_x10 % 10));
            snprintf(sensor_humidity, sizeof(sensor_humidity), "%u.%u %%",
                     (unsigned int)(humidity_x10 / 10U),
                     (unsigned int)(humidity_x10 % 10U));
            sensor_ready = 1U;
            taskEXIT_CRITICAL();
        }
        osDelay(2000U);
    }
}

uint8_t SensorTask_GetLatest(char *temperature, uint16_t temperature_size,
                             char *humidity, uint16_t humidity_size)
{
    if(!temperature || !humidity || temperature_size == 0U || humidity_size == 0U) return 1U;
    taskENTER_CRITICAL();
    if(!sensor_ready) {
        taskEXIT_CRITICAL();
        return 1U;
    }
    strncpy(temperature, sensor_temperature, temperature_size - 1U);
    temperature[temperature_size - 1U] = '\0';
    strncpy(humidity, sensor_humidity, humidity_size - 1U);
    humidity[humidity_size - 1U] = '\0';
    taskEXIT_CRITICAL();
    return 0U;
}
