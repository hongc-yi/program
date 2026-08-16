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
volatile const char *g_over_task_name = 0; // 栈溢出检测任务名称
volatile TaskHandle_t g_over_task_handle = 0; // 栈溢出检测任务句柄

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
// 检测任务栈高水位句柄
osThreadId_t checkStackHighWatermarkHandle;
const osThreadAttr_t checkStackHighWatermark_attributes = {
  .name = "checkStackHighWatermark",
  .stack_size = 512 * 4, // printf + 256B buffer + 运行时统计格式化，512B 会溢出
  .priority = (osPriority_t) osPriorityAboveNormal, // 高于 defaultTask，printf 不被时间片轮转打断（否则串口字节流被切断导致乱码）
};
// 检测任务栈高水位任务
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 1,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void keyTaskFuc(void *argument); // 按键发送信号量任务
void logTaskFuc(void *argument); // 轮询按键信号量，获取按键状态
void vApplicationTickHook(void); // 时间钩子函数
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName ); // 栈溢出检测任务
void checkStackHighWatermarkTaskFuc(void *argument); // 检测各种系统状态任务(有栈高水位、堆剩余空间、CPU占用率)
void testAssert(const char *str); // 测试断言函数
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
  // log 任务（当前用于测试断言）
  logTaskHandle = osThreadNew(logTaskFuc, NULL, &logTask_attributes);
  // 检测任务栈高水位任务
  checkStackHighWatermarkHandle = osThreadNew(checkStackHighWatermarkTaskFuc, NULL, &checkStackHighWatermark_attributes);
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
  // 模拟栈溢出：任务栈仅 128B，256B 数组 + 写满即可越界破坏相邻内存
  // （不要用超大数组如 1MB，SP 会直接跳出 128KB RAM 导致 HardFault，检测钩子来不及跑）
  // volatile 防止编译器把"写了没读"的数组优化掉，导致溢出模拟失效
  // 以下代码用于测试栈溢出检测钩子
  // volatile uint8_t stackOverflowArray[256];
  // uint32_t i;
  // for(i = 0; i < sizeof(stackOverflowArray); i++){
  //   stackOverflowArray[i] = (uint8_t)i; // 写满数组，触发栈溢出
  // }
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
    printf("thread logTask!\r\n");
    testAssert("thread logTask");
    testAssert(NULL); // 测试断言，触发栈溢出检测钩子
    osDelay(1000); // 1秒打印一次

    
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

// 测试断言：自定义断言，失败时打印信息再停（configASSERT 失败是无声死循环，看不到任何输出）
void testAssert(const char *str)
{
  if(str == NULL){
    printf("assert failed: str is NULL!\r\n");
    // 先打印，再关中断+死循环：只 while(1) 会停不住系统（高优先级任务照常抢占运行）
    taskDISABLE_INTERRUPTS(); // 停 SysTick/PendSV，调度器停转，整个系统冻结
    while(1);
  }
  printf("testAssert: %s\r\n", str);
}

// 时间钩子函数实现呼吸灯
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

// 钩子函数栈溢出检测,如果栈溢出则会跳到这里，打印溢出任务名称和句柄，方便调试
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
  /* USER CODE BEGIN vApplicationStackOverflowHook */
  g_over_task_name = pcTaskName; // 保存溢出任务名称
  g_over_task_handle = xTask; // 保存溢出任务句柄

  // 注意：此时任务栈已损坏，钩子里尽量少用栈。
  // 删除了 _BKPT（无调试器连接时 BKPT 指令直接 HardFault，printf 来不及执行）；
  // 删除了关中断（下面的死循环已足够停止系统）
  printf("Stack overflow in task: %s, handle: %lx\r\n", pcTaskName, (unsigned long)xTask);
  while(1);
  /* USER CODE END vApplicationStackOverflowHook */
}

// 检测各种系统状态任务(有栈高水位、堆剩余空间、CPU占用率)
void checkStackHighWatermarkTaskFuc(void *argument)
{
  uint32_t stackrest;
  uint32_t heaprest;
  char buffer[256];
  /* USER CODE BEGIN checkStackHighWatermarkTask */
  /* Infinite loop */
  for(;;)
  {
    // 钩子函数运行在 SysTick 中断的 MSP 栈上，没有自己的任务栈，
    // osThreadGetStackSpace 只能查任务；注意本版 CubeMX 移植不支持 NULL=当前任务（会返回 0），需传真实句柄
    stackrest = osThreadGetStackSpace(defaultTaskHandle);
    printf("defaultTask 栈高水位: %d 字节\r\n", stackrest);
    stackrest = osThreadGetStackSpace(checkStackHighWatermarkHandle);
    printf("check 任务自身栈高水位: %d 字节\r\n", stackrest);

    // 检测堆剩余空间
    heaprest = xPortGetFreeHeapSize();
    printf("总空间: 15360 字节\r\n");
    printf("堆剩余空间: %d 字节\r\n", heaprest);

    // 检测CPU占用率：CPU占用率 = 1 - (空闲任务剩余栈空间 / 空闲任务总栈空间)
    vTaskGetRunTimeStats(buffer);
    printf("CPU占用率: %s\r\n", buffer);

    osDelay(1000);
  }
  /* USER CODE END checkStackHighWatermarkTask */
}
/* USER CODE END Application */

