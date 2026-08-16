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
// 高优先级任务：与低优先级任务抢同一把互斥锁
osThreadId_t task_HHandle;
const osThreadAttr_t task_H_attributes = {
  .name = "task_H",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow2,
};
// 中优先级任务：不抢锁，观察锁被占用时它的调度情况
osThreadId_t task_MHandle;
const osThreadAttr_t task_M_attributes = {
  .name = "task_M",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow1,
};
// 低优先级任务：与高优先级任务抢同一把互斥锁
osThreadId_t task_LHandle;
const osThreadAttr_t task_L_attributes = {
  .name = "task_L",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
// 互斥锁句柄
osMutexId_t Mutex01Handle;
const osMutexAttr_t Mutex01_attributes = {
  .name = "Mutex01"
};
// 二值信号量句柄（本演示未使用，保留创建以对比互斥锁）
osSemaphoreId_t BinarySem01Handle;
const osSemaphoreAttr_t BinarySem01_attributes = {
  .name = "BinarySem01"
};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void Task_H_Func(void *argument);
void Task_M_Func(void *argument);
void Task_L_Func(void *argument);
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
  // 创建互斥锁（供高/低优先级任务争夺）
  Mutex01Handle = osMutexNew(&Mutex01_attributes);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  // 创建二值信号量（仅保留创建，本演示用互斥锁）
  BinarySem01Handle = osSemaphoreNew(1, 1, &BinarySem01_attributes);
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
  // 创建三个任务，高/低任务抢同一把互斥锁
  task_HHandle = osThreadNew(Task_H_Func, NULL, &task_H_attributes);
  task_MHandle = osThreadNew(Task_M_Func, NULL, &task_M_attributes);
  task_LHandle = osThreadNew(Task_L_Func, NULL, &task_L_attributes);
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
    HAL_GPIO_TogglePin(LED_T_GPIO_Port, LED_T_Pin);
    osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
// 高优先级任务：抢锁后忙等 1 秒（HAL_Delay 不释放 CPU），演示互斥独占
void Task_H_Func(void *argument)
{
  /* USER CODE BEGIN Task_H_Func */
  /* Infinite loop */
  for(;;)
  {
    // 获取互斥锁（被占用则阻塞等待）
    // osSemaphoreAcquire(BinarySem01Handle, osWaitForever);
    osMutexAcquire(Mutex01Handle, osWaitForever);
    printf("High Task get mutex, start\r\n");
    // HAL_Delay 忙等模拟持锁工作，期间其他任务无法获得 CPU
    HAL_Delay(1000);
    printf("High Task give mutex, end\r\n");
    // 释放互斥锁
    // osSemaphoreRelease(BinarySem01Handle);
    osMutexRelease(Mutex01Handle);
    osDelay(1000);
  }
  /* USER CODE END Task_H_Func */
}

// 中优先级任务：不抢锁，观察锁被占用时它是否还能运行
void Task_M_Func(void *argument)
{
  /* USER CODE BEGIN Task_M_Func */
  /* Infinite loop */
  for(;;)
  {
    printf("Middle Task use cpu, but do nothing\r\n");
    osDelay(1000);
  }
  /* USER CODE END Task_M_Func */
}

// 低优先级任务：与高优先级任务抢同一把互斥锁，忙等 3 秒
void Task_L_Func(void *argument)
{
  /* USER CODE BEGIN Task_L_Func */
  /* Infinite loop */
  for(;;)
  {
    // 获取互斥锁（被占用则阻塞等待）
    // osSemaphoreAcquire(BinarySem01Handle, osWaitForever);
    osMutexAcquire(Mutex01Handle, osWaitForever);
    printf("Low Task get mutex, start\r\n");
    // HAL_Delay 忙等模拟持锁工作，期间其他任务无法获得 CPU
    HAL_Delay(3000);
    printf("Low Task give mutex, end\r\n");
    // 释放互斥锁
    // osSemaphoreRelease(BinarySem01Handle);
    osMutexRelease(Mutex01Handle);
    osDelay(1000);
  }
  /* USER CODE END Task_L_Func */
}
/* USER CODE END Application */
