#include "key.h"
#include "delay.h"

static volatile uint8_t key_pending = 0;

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
        key_pending = 1;
        last_irq_tick = now;
    }
}

uint8_t Key_Get_Pending(void)
{
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

