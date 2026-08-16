/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "key.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
// 按键发送信号量任务句柄
osThreadId_t keyTaskHandle;
const osThreadAttr_t keyTask_attributes = {
  .name = "keyTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow2,
};
// 轮询按键信号量发送log任务句柄
osThreadId_t logTaskHandle;
const osThreadAttr_t logTask_attributes = {
  .name = "logTask",
  .stack_size = 256 * 4, // printf 需要较大栈，512B 易溢出
  .priority = (osPriority_t) osPriorityLow1,
};
// 按键信号量句柄
osSemaphoreId_t KeyBinarySem01Handle;
const osSemaphoreAttr_t KeyBinarySem01_attributes = {
  .name = "KeyBinarySem01"
};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void keyTaskFuc(void *argument); // 按键发送信号量任务
void logTaskFuc(void *argument); // 轮询按键信号量，获取按键状态
void vApplicationIdleHook(void); // 空闲任务钩子函数
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  // 创建按键信号量（二值信号量，初始值 0：无按键事件）
  KeyBinarySem01Handle = osSemaphoreNew(1, 0, &KeyBinarySem01_attributes);
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  // 创建按键任务（返回值赋给任务句柄，勿覆盖信号量句柄）
  keyTaskHandle = osThreadNew(keyTaskFuc, NULL, &keyTask_attributes);
  // 创建log任务
  logTaskHandle = osThreadNew(logTaskFuc, NULL, &logTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
// 按键任务
void keyTaskFuc(void *argument)
{
  /* USER CODE BEGIN keyTask */
  /* Infinite loop */
  for(;;)
  {
    if(KeyScan(0)){
      // 按键按下，释放信号量
      osSemaphoreRelease(KeyBinarySem01Handle);
    }
    osDelay(1);
  }
  /* USER CODE END keyTask */
}
// 轮询按键信号量，获取按键状态
void logTaskFuc(void *argument)
{
  /* USER CODE BEGIN logTask */
  /* Infinite loop */
  for(;;) 
  {
    // 等待按键信号量
    if(osSemaphoreAcquire(KeyBinarySem01Handle, osWaitForever) == osOK){
      // 按键按下，打印log
      printf("keyTask!\r\n");
    }
    // 不给资源，避免占用CPU
    osDelay(1);
  }
  /* USER CODE END logTask */
}

void vApplicationIdleHook(void)
{
  /* USER CODE BEGIN vApplicationIdleHook */
  // 空闲任务钩子函数
  // 可以在这里做一些低功耗处理
  // 不可以做阻塞操作，否则会影响系统运行
  /* USER CODE END vApplicationIdleHook */
  timeCounter += 1;
  if(timeCounter >= 500){
    timeCounter = 0;
    // 切换LED状态
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    // 打印log
    printf("IdleTask!\r\n");
  }
}
/* USER CODE END Application */

