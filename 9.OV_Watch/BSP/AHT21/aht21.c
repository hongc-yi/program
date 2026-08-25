#include "aht21.h"
#include "stm32f4xx_hal.h"
#include "delay.h"

#define AHT21_ADDR 0x38U
#define AHT21_SDA_PORT GPIOC
#define AHT21_SCL_PORT GPIOC
#define AHT21_SDA_PIN GPIO_PIN_5
#define AHT21_SCL_PIN GPIO_PIN_4

static int16_t aht21_temperature_x10 = 0;
static uint16_t aht21_humidity_x10 = 0;
static uint8_t aht21_ready = 0;

typedef enum {
    AHT21_SDA_INPUT = 0,
    AHT21_SDA_OUTPUT = 1
} aht21_sda_mode_t;

static void aht21_sda_mode(aht21_sda_mode_t mode)
{
    GPIO_InitTypeDef init = {0};

    init.Pin = AHT21_SDA_PIN;
    init.Pull = GPIO_PULLUP;
    init.Speed = GPIO_SPEED_FREQ_HIGH;
    init.Mode = (mode == AHT21_SDA_OUTPUT) ? GPIO_MODE_OUTPUT_OD : GPIO_MODE_INPUT;
    HAL_GPIO_Init(AHT21_SDA_PORT, &init);
}

static void aht21_sda_write(GPIO_PinState state)
{
    HAL_GPIO_WritePin(AHT21_SDA_PORT, AHT21_SDA_PIN, state);
}

static void aht21_scl_write(GPIO_PinState state)
{
    HAL_GPIO_WritePin(AHT21_SCL_PORT, AHT21_SCL_PIN, state);
}

static GPIO_PinState aht21_sda_read(void)
{
    return HAL_GPIO_ReadPin(AHT21_SDA_PORT, AHT21_SDA_PIN);
}

static void aht21_start(void)
{
    aht21_sda_mode(AHT21_SDA_OUTPUT);
    aht21_sda_write(GPIO_PIN_SET);
    aht21_scl_write(GPIO_PIN_SET);
    delay_us(2);
    aht21_sda_write(GPIO_PIN_RESET);
    delay_us(2);
    aht21_scl_write(GPIO_PIN_RESET);
}

static void aht21_stop(void)
{
    aht21_sda_mode(AHT21_SDA_OUTPUT);
    aht21_sda_write(GPIO_PIN_RESET);
    aht21_scl_write(GPIO_PIN_SET);
    delay_us(2);
    aht21_sda_write(GPIO_PIN_SET);
    delay_us(2);
}

static uint8_t aht21_wait_ack(void)
{
    uint8_t timeout = 0;

    aht21_sda_mode(AHT21_SDA_INPUT);
    aht21_scl_write(GPIO_PIN_SET);
    while(aht21_sda_read() == GPIO_PIN_SET) {
        delay_us(1);
        if(++timeout >= 20U) {
            aht21_scl_write(GPIO_PIN_RESET);
            return 1U;
        }
    }
    aht21_scl_write(GPIO_PIN_RESET);
    return 0U;
}

static void aht21_write_bit(uint8_t bit)
{
    aht21_sda_mode(AHT21_SDA_OUTPUT);
    aht21_sda_write(bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
    delay_us(1);
    aht21_scl_write(GPIO_PIN_SET);
    delay_us(2);
    aht21_scl_write(GPIO_PIN_RESET);
}

static void aht21_write_byte(uint8_t value)
{
    uint8_t bit;
    for(bit = 0; bit < 8U; bit++) {
        aht21_write_bit((value & 0x80U) != 0U);
        value <<= 1;
    }
}

static uint8_t aht21_read_bit(void)
{
    GPIO_PinState bit;
    aht21_sda_mode(AHT21_SDA_INPUT);
    aht21_scl_write(GPIO_PIN_SET);
    delay_us(2);
    bit = aht21_sda_read();
    aht21_scl_write(GPIO_PIN_RESET);
    return bit == GPIO_PIN_SET ? 1U : 0U;
}

static uint8_t aht21_read_byte(uint8_t ack)
{
    uint8_t value = 0;
    uint8_t bit;

    for(bit = 0; bit < 8U; bit++) {
        value = (uint8_t)((value << 1) | aht21_read_bit());
    }

    aht21_sda_mode(AHT21_SDA_OUTPUT);
    aht21_sda_write(ack ? GPIO_PIN_RESET : GPIO_PIN_SET);
    aht21_scl_write(GPIO_PIN_SET);
    delay_us(1);
    aht21_scl_write(GPIO_PIN_RESET);
    return value;
}

static uint8_t aht21_crc8(const uint8_t *data, uint8_t length)
{
    uint8_t crc = 0xFFU;
    uint8_t i;
    uint8_t bit;

    for(i = 0; i < length; i++) {
        crc ^= data[i];
        for(bit = 0; bit < 8U; bit++) {
            crc = (crc & 0x80U) ? (uint8_t)((crc << 1) ^ 0x31U) : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

uint8_t AHT21_Init(void)
{
    GPIO_InitTypeDef init = {0};
    uint8_t status;

    __HAL_RCC_GPIOC_CLK_ENABLE();
    init.Pin = AHT21_SCL_PIN;
    init.Mode = GPIO_MODE_OUTPUT_OD;
    init.Pull = GPIO_PULLUP;
    init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(AHT21_SCL_PORT, &init);
    aht21_sda_mode(AHT21_SDA_OUTPUT);
    aht21_sda_write(GPIO_PIN_SET);
    aht21_scl_write(GPIO_PIN_SET);
    delay_ms(5);

    /* Read status. If the calibration bit is clear, initialize the sensor. */
    aht21_start();
    aht21_write_byte((uint8_t)(AHT21_ADDR << 1));
    if(aht21_wait_ack()) {
        aht21_stop();
        return 1U;
    }
    aht21_write_byte(0x71U);
    if(aht21_wait_ack()) {
        aht21_stop();
        return 1U;
    }
    aht21_stop();

    aht21_start();
    aht21_write_byte((uint8_t)((AHT21_ADDR << 1) | 1U));
    if(aht21_wait_ack()) {
        aht21_stop();
        return 1U;
    }
    status = aht21_read_byte(0U);
    aht21_stop();

    if((status & 0x08U) == 0U) {
        aht21_start();
        aht21_write_byte((uint8_t)(AHT21_ADDR << 1));
        if(aht21_wait_ack()) {
            aht21_stop();
            return 1U;
        }
        aht21_write_byte(0xBEU);
        if(aht21_wait_ack()) {
            aht21_stop();
            return 1U;
        }
        aht21_write_byte(0x08U);
        if(aht21_wait_ack()) {
            aht21_stop();
            return 1U;
        }
        aht21_write_byte(0x00U);
        if(aht21_wait_ack()) {
            aht21_stop();
            return 1U;
        }
        aht21_stop();
        delay_ms(10);
    }

    aht21_ready = 1U;
    return 0U;
}

uint8_t AHT21_Read(int16_t *temperature_x10, uint16_t *humidity_x10)
{
    uint8_t data[7];
    uint32_t humidity_raw;
    uint32_t temperature_raw;
    uint8_t i;

    if(!temperature_x10 || !humidity_x10) return 1U;
    if(!aht21_ready && AHT21_Init()) return 1U;

    aht21_start();
    aht21_write_byte((uint8_t)(AHT21_ADDR << 1));
    if(aht21_wait_ack()) {
        aht21_stop();
        return 1U;
    }
    aht21_write_byte(0xACU);
    if(aht21_wait_ack()) {
        aht21_stop();
        return 1U;
    }
    aht21_write_byte(0x33U);
    if(aht21_wait_ack()) {
        aht21_stop();
        return 1U;
    }
    aht21_write_byte(0x00U);
    if(aht21_wait_ack()) {
        aht21_stop();
        return 1U;
    }
    aht21_stop();
    delay_ms(80);

    aht21_start();
    aht21_write_byte((uint8_t)((AHT21_ADDR << 1) | 1U));
    if(aht21_wait_ack()) {
        aht21_stop();
        return 1U;
    }
    for(i = 0; i < 7U; i++) data[i] = aht21_read_byte(i < 6U ? 1U : 0U);
    aht21_stop();

    if((data[0] & 0x80U) != 0U || aht21_crc8(data, 6U) != data[6]) return 1U;

    humidity_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | ((uint32_t)data[3] >> 4);
    temperature_raw = ((uint32_t)(data[3] & 0x0FU) << 16) | ((uint32_t)data[4] << 8) | data[5];
    *humidity_x10 = (uint16_t)((humidity_raw * 1000U) / 1048576U);
    *temperature_x10 = (int16_t)((temperature_raw * 2000U) / 1048576U - 500);
    aht21_temperature_x10 = *temperature_x10;
    aht21_humidity_x10 = *humidity_x10;
    return 0U;
}

uint8_t AHT21_GetLatest(int16_t *temperature_x10, uint16_t *humidity_x10)
{
    if(!temperature_x10 || !humidity_x10 || !aht21_ready) return 1U;
    *temperature_x10 = aht21_temperature_x10;
    *humidity_x10 = aht21_humidity_x10;
    return 0U;
}
