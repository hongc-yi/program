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
// === 呼吸灯参数（tick 钩子软件 PWM）===
// PC13 无定时器通道，硬件 PWM 不可用，用钩子每 tick 翻转 IO 模拟
#define PWM_PERIOD   20    // PWM 周期 = 20 tick = 20ms（50Hz，人眼取亮度平均）
#define BREATH_STEPS 20    // 占空比级数（0~20）
static uint16_t pwmTick = 0;     // PWM 周期内 tick 计数
static uint8_t  breathDuty = 0;  // 当前占空比（0~20）
static uint8_t  brightening = 1; // 呼吸方向：1 变亮 / 0 变暗

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
void vApplicationTickHook(void); // 时间钩子函数
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
  // 时间钩子 vApplicationTickHook 由内核在每个时间周期后自动调用，无需（也不可）osThreadNew 创建
  //keyTaskHandle = osThreadNew(keyTaskFuc, NULL, &keyTask_attributes);
  //logTaskHandle = osThreadNew(logTaskFuc, NULL, &logTask_attributes);
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
    // LED 已由 tick 钩子做呼吸灯驱动，本任务空转维持系统负载
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

void vApplicationTickHook(void)
{
  /* USER CODE BEGIN vApplicationTickHook */
  // tick钩子函数：在 SysTick 中断上下文执行，必须极短（只做计数和 GPIO 写）
  /* USER CODE END vApplicationTickHook */
  // 1. PWM 周期计数
  pwmTick++;
  if(pwmTick >= PWM_PERIOD){
    pwmTick = 0;
    // 2. 每个 PWM 周期改变一级占空比（呼吸方向翻转）
    if(brightening){
      breathDuty++;
      if(breathDuty >= BREATH_STEPS) brightening = 0;
    }else{
      breathDuty--;
      if(breathDuty == 0) brightening = 1;
    }
  }
  // 3. 软件 PWM 输出：周期内前 breathDuty 个 tick 点亮（PC13 低电平点亮）
  if(pwmTick < breathDuty)
    HAL_GPIO_WritePin(LED_T_GPIO_Port, LED_T_Pin, GPIO_PIN_RESET); // 亮
  else
    HAL_GPIO_WritePin(LED_T_GPIO_Port, LED_T_Pin, GPIO_PIN_SET);   // 灭
}
/* USER CODE END Application */

