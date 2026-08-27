#include "key.h"
#include "delay.h"

static volatile uint8_t key_pending = 0;
static volatile uint8_t key_irq_flag = 0;

void Key_Port_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = KEY1_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY1_PORT, &GPIO_InitStruct);


	
  /* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

void Key_Interrupt_Callback(void)
{
    static uint32_t last_irq_tick = 0;
    uint32_t now = HAL_GetTick();

    if((now - last_irq_tick) >= 120U) {
        last_irq_tick = now;
        key_irq_flag = 1;   /* 只做标记，电平确认放到任务上下文做 */
    }
}

uint8_t Key_Get_Pending(void)
{
    if(key_irq_flag) {
        key_irq_flag = 0;
        /* 电平确认滤波：串扰/上电噪声产生的瞬时下降沿，此刻电平往往已回高，直接丢弃 */
        delay_ms(20);
        if(KEY1 == GPIO_PIN_RESET) {
            key_pending = 1;
        }
    }
    uint8_t pending = key_pending;
    key_pending = 0;
    return pending;
}

uint8_t KeyScan(uint8_t mode)
{
	static uint8_t key_up = 1;
	uint8_t keyvalue=0;
	if(mode) key_up = 1;
	if( key_up && (!KEY1))
	{
		delay_ms(3);//ensure the key is down
		if(!KEY1) keyvalue = 1;
		if(keyvalue) key_up = 0;
	}
	else
	{
		delay_ms(3);//ensure the key is up
		if(KEY1)
			key_up = 1;
	}
	return keyvalue;
}

