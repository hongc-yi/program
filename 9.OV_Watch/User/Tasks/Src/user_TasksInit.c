/* Private includes -----------------------------------------------------------*/
//includes
#include "user_TasksInit.h"
//sys
#include "sys.h"
#include "stdio.h"

//bsp
#include "key.h"

//gui
#include "lvgl.h"

//tasks
#include "user_HardwareInitTask.h"
#include "user_LVGLTask.h"
#include "user_SensorTask.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Timers --------------------------------------------------------------------*/


/* Tasks ---------------------------------------------------------------------*/
// Hardwares initialization
osThreadId_t HardwareInitTaskHandle;
const osThreadAttr_t HardwareInitTask_attributes = {
  .name = "HardwareInitTask",
  .stack_size = 128 * 20,
  .priority = (osPriority_t) osPriorityHigh,
};

// LVGL Handler task
osThreadId_t LvHandlerTaskHandle;
const osThreadAttr_t LvHandlerTask_attributes = {
  .name = "LvHandlerTask",
  .stack_size = 128 * 32,
  .priority = (osPriority_t) osPriorityLow1,
};

osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
  .name = "SensorTask",
  .stack_size = 128 * 6,
  .priority = (osPriority_t) osPriorityBelowNormal,
};


/* Message queues ------------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
void LvHandlerTask(void *argument);

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void User_Tasks_Init(void)
{
  /* add mutexes, ... */

  /* add semaphores, ... */

  /* start timers, add new ones, ... */

  /* add queues, ... */

	/* add threads, ... */
  HardwareInitTaskHandle  = osThreadNew(HardwareInitTask, NULL, &HardwareInitTask_attributes);
  LvHandlerTaskHandle     = osThreadNew(LvHandlerTask, NULL, &LvHandlerTask_attributes);
  SensorTaskHandle        = osThreadNew(SensorTask, NULL, &SensorTask_attributes);

  /* add events, ... */

	/* add  others ... */


}

