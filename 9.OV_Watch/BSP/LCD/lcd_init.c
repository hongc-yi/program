#include "lcd_init.h"
#include "delay.h"
#include "spi.h"

#define LCD_BRIGHTNESS_DEFAULT 20U
#define LCD_BACKLIGHT_PERIOD 999U

static TIM_HandleTypeDef lcd_backlight_timer;
static uint8_t lcd_backlight_pwm_ready = 0;
static uint8_t lcd_backlight_percent = LCD_BRIGHTNESS_DEFAULT;

static void LCD_Backlight_Init(void)
{
    GPIO_InitTypeDef gpio_init = {0};
    TIM_OC_InitTypeDef oc_config = {0};

    __HAL_RCC_TIM2_CLK_ENABLE();

    gpio_init.Pin = BLK_PIN;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_init.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(BLK_PORT, &gpio_init);

    lcd_backlight_timer.Instance = TIM2;
    lcd_backlight_timer.Init.Prescaler = 99;
    lcd_backlight_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
    lcd_backlight_timer.Init.Period = LCD_BACKLIGHT_PERIOD;
    lcd_backlight_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    lcd_backlight_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if(HAL_TIM_PWM_Init(&lcd_backlight_timer) != HAL_OK) return;

    oc_config.OCMode = TIM_OCMODE_PWM1;
    oc_config.Pulse = (LCD_BACKLIGHT_PERIOD + 1U) * LCD_BRIGHTNESS_DEFAULT / 100U;
    oc_config.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc_config.OCFastMode = TIM_OCFAST_DISABLE;

    if(HAL_TIM_PWM_ConfigChannel(&lcd_backlight_timer, &oc_config, TIM_CHANNEL_1) != HAL_OK) return;
    if(HAL_TIM_PWM_Start(&lcd_backlight_timer, TIM_CHANNEL_1) != HAL_OK) return;

    lcd_backlight_pwm_ready = 1;
}

/******************************************************************************
      函数说明：LCD端口初始化
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure = {0};
	
	__HAL_RCC_GPIOC_CLK_ENABLE();
 	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	LCD_Backlight_Init();	 
 	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; 		 //推挽输出
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//速度50MHz
 	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);	  //初始化GPIOA
 	
	
	GPIO_InitStructure.Pin = RES_PIN;	 
 	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; 		 //推挽输出
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//速度50MHz
 	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	  //初始化GPIOB
 	HAL_GPIO_WritePin(GPIOB, RES_PIN, GPIO_PIN_SET);

	GPIO_InitStructure.Pin = DC_PIN;	 
 	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; 		 //推挽输出
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//速度50MHz
 	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);	  //初始化GPIOC
 	HAL_GPIO_WritePin(GPIOC, DC_PIN, GPIO_PIN_SET);

	GPIO_InitStructure.Pin = CS_PIN;	 
 	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP; 		 //推挽输出
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//速度50MHz
 	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);	  //初始化GPIOD
 	HAL_GPIO_WritePin(GPIOD, CS_PIN, GPIO_PIN_SET);

}


/******************************************************************************
      函数说明：LCD串行数据写入函数(software SPI)
      入口数据：dat  要写入的串行数据
      返回值：  无
******************************************************************************/
void LCD_Writ_Bus(u8 dat) 
{	
	//hard SPI
	HAL_SPI_Transmit(&hspi1,&dat,1,1);
	
	//soft SPI
	/*
	u8 i;
	for(i=0;i<8;i++)
	{			  
		LCD_SCLK_Clr();
		if(dat&0x80)
		{
		   LCD_MOSI_Set();
		}
		else
		{
		   LCD_MOSI_Clr();
		}
		LCD_SCLK_Set();
		dat<<=1;
	}	
	*/
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA8(u8 dat)
{
	LCD_Writ_Bus(dat);
}


/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(u16 dat)
{
//	LCD_Writ_Bus(dat>>8);
//	LCD_Writ_Bus(dat);
	uint8_t temp[2];
	temp[0]=(dat>>8)&0xff;
	temp[1]=dat&0xff;
	HAL_SPI_Transmit(&hspi1,temp,2,1);
	
}


/******************************************************************************
      函数说明：LCD写入命令
      入口数据：dat 写入的命令
      返回值：  无
******************************************************************************/
void LCD_WR_REG(u8 dat)
{
	LCD_DC_Clr();//写命令
	LCD_Writ_Bus(dat);
	LCD_DC_Set();//写数据
}


/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2)
{
	LCD_WR_REG(0x2a);//列地址设置
	LCD_WR_DATA(x1);
	LCD_WR_DATA(x2);
	LCD_WR_REG(0x2b);//行地址设置
	LCD_WR_DATA(y1);
	LCD_WR_DATA(y2);
	LCD_WR_REG(0x2c);//储存器写
}


/******************************************************************************
      函数说明：LCD调节背光
      入口数据：dc,占空比,5%~100%
      返回值：  无
******************************************************************************/
void LCD_Set_Light(uint8_t dc)
{
    if(dc > 100U) dc = 100U;
    lcd_backlight_percent = dc;

    if(lcd_backlight_pwm_ready)
    {
        uint32_t pulse = (LCD_BACKLIGHT_PERIOD + 1U) * dc / 100U;
        __HAL_TIM_SET_COMPARE(&lcd_backlight_timer, TIM_CHANNEL_1, pulse);
    }
}

uint8_t LCD_Get_Light(void)
{
    return lcd_backlight_percent;
}


/******************************************************************************
      函数说明：LCD关闭背光
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_Close_Light(void)
{
    LCD_Set_Light(0);
}


/******************************************************************************
      函数说明：LCD开启背光
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_Open_Light(void)
{
    if(lcd_backlight_percent == 0U) lcd_backlight_percent = LCD_BRIGHTNESS_DEFAULT;
    LCD_Set_Light(lcd_backlight_percent);
}

/******************************************************************************
      函数说明：ST7789 SLEEP IN
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_ST7789_SleepIn(void)
{
	LCD_WR_REG(0x10);
	delay_ms(100);
}


/******************************************************************************
      函数说明：ST7789 SLEEP OUT
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_ST7789_SleepOut(void)
{
	LCD_WR_REG(0x11);
	delay_ms(100);
}


/******************************************************************************
      函数说明：LCD初始化
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_Init(void)
{
	LCD_GPIO_Init();//初始化GPIO
	LCD_CS_Clr();		//chip select
	
	LCD_RES_Clr();	//复位
	delay_ms(100);
	LCD_RES_Set();
	delay_ms(100);
	
	LCD_WR_REG(0x11); 
	delay_ms(120); 
	LCD_WR_REG(0x36); 
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
	else LCD_WR_DATA8(0xA0);

	LCD_WR_REG(0x3A);
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB2);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x00);
	LCD_WR_DATA8(0x33);
	LCD_WR_DATA8(0x33); 

	LCD_WR_REG(0xB7); 
	LCD_WR_DATA8(0x35);  

	LCD_WR_REG(0xBB);
	LCD_WR_DATA8(0x19);

	LCD_WR_REG(0xC0);
	LCD_WR_DATA8(0x2C);

	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);
	LCD_WR_DATA8(0x12);   

	LCD_WR_REG(0xC4);
	LCD_WR_DATA8(0x20);  

	LCD_WR_REG(0xC6); 
	LCD_WR_DATA8(0x0F);    

	LCD_WR_REG(0xD0); 
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1);

	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2B);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x54);
	LCD_WR_DATA8(0x4C);
	LCD_WR_DATA8(0x18);
	LCD_WR_DATA8(0x0D);
	LCD_WR_DATA8(0x0B);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x23);

	LCD_WR_REG(0xE1);
	LCD_WR_DATA8(0xD0);
	LCD_WR_DATA8(0x04);
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x11);
	LCD_WR_DATA8(0x13);
	LCD_WR_DATA8(0x2C);
	LCD_WR_DATA8(0x3F);
	LCD_WR_DATA8(0x44);
	LCD_WR_DATA8(0x51);
	LCD_WR_DATA8(0x2F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x1F);
	LCD_WR_DATA8(0x20);
	LCD_WR_DATA8(0x23);

	/* The P169H002 panel needs inversion enabled for normal colors. */
	LCD_WR_REG(0x21);

	LCD_WR_REG(0x29); 
}



