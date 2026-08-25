/* Private includes -----------------------------------------------------------*/
//includes

//bsp

// tasks
#include "user_TasksInit.h"
#include "user_LVGLTask.h"
#include "key.h"
#include "esp_link.h"
#include "user_SensorTask.h"

//gui
#include "lvgl.h"
#include "ui.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  FreeRTOS Tick Hook, to increase the LVGL tick
  * @param  None
  * @retval None
  */
void TaskTickHook(void)
{
	//to increase the LVGL tick
	lv_tick_inc(1);
}


/**
  * @brief  LVGL Handler task, to run the lvgl
  * @param  argument: Not used
  * @retval None
  */
void LvHandlerTask(void *argument)
{
  uint32_t _time = 1; // default delay time
  char temperature[16];
  char humidity[16];
  while(1)
  {
    ESP_Link_Process();
    if(SensorTask_GetLatest(temperature, sizeof(temperature), humidity, sizeof(humidity)) == 0U) {
      ui_set_sensor_values(temperature, humidity);
    }
    if(Key_Get_Pending()) {
      ui_handle_key_confirm();
    }
		_time = lv_timer_handler();
    // 限制最大休眠时间，保证按键队列被及时读取
    if(_time > 30) {
        _time = 30;
    }
    osDelay(_time);
	}
}
