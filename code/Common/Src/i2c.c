#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
//#include "stm32f4xx_conf.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_ll_gpio.h"

#include "i2c.h"

void set_scl(i2c_config_t *pI2c, int v)
{
  if (v) {
    LL_GPIO_SetOutputPin( pI2c->sclGpio, (1 << pI2c->sclPin) );
  } else {
    LL_GPIO_ResetOutputPin( pI2c->sclGpio, (1 << pI2c->sclPin) );
  }
}
void set_sda(i2c_config_t *pI2c, int v)
{
  if (v) {
    LL_GPIO_SetOutputPin( pI2c->sdaGpio, (1 << pI2c->sdaPin) );
  } else {
    LL_GPIO_ResetOutputPin( pI2c->sdaGpio, (1 << pI2c->sdaPin) );
  }
}
uint8_t get_sda(i2c_config_t *pI2c)
{
  return LL_GPIO_IsInputPinSet(pI2c->sdaGpio, 1 << pI2c->sdaPin);
  //GPIO_ReadInputDataBit( pI2c->sdaGpio, pI2c->sdaPin);
}

void i2c_start(i2c_config_t *pI2c)
{
  set_sda(pI2c,1);
  wait_us(pI2c->delay_us);
  set_scl(pI2c,1);
  wait_us(pI2c->delay_us);
  set_sda(pI2c,0);
  wait_us(pI2c->delay_us);
  set_scl(pI2c,0);
}
void i2c_stop(i2c_config_t *pI2c)
{
  set_scl(pI2c,0);
  set_sda(pI2c,0);
  wait_us(pI2c->delay_us);
  set_scl(pI2c,1);
  wait_us(pI2c->delay_us);
  set_sda(pI2c,1);
}

uint16_t i2c_xfer(i2c_config_t *pI2c, uint16_t in)
{
  uint16_t out = 0;
  int i;

  for (i = 0;i < 9;i++) {
    if ((in << i) & 0x100) {
      set_sda(pI2c,1);
    } else {
      set_sda(pI2c,0);
    }
    wait_us(pI2c->delay_us);
    set_scl(pI2c,1);
    wait_us(pI2c->delay_us);
    if (get_sda(pI2c)) {
      out = (out << 1) | 1;
    } else {
      out = out << 1;
    }
    set_scl(pI2c,0);
    wait_us(pI2c->delay_us);
  }
  return out;
}

void i2c_read(i2c_config_t *pI2c, uint8_t address, uint8_t *pData, int length)
{
  int i;
  uint16_t r;

  i2c_start(pI2c);
  r = i2c_xfer(pI2c,(address << 1) | 3);
  if (r & 1) {
    i2c_stop(pI2c);
    return;
  }
  for (i = 0;i < length;i++) {
    if (i == length-1) {
      r = i2c_xfer(pI2c,0x1FF);
    } else {
      r = i2c_xfer(pI2c,0x1FE);
    }
    pData[i] = r >> 1;
  }
  i2c_stop(pI2c);
}

void i2c_write(i2c_config_t *pI2c, uint8_t address,
	       uint8_t *pData_write, int length_write)
{
  int i;
  uint16_t r;

  i2c_start(pI2c);
  r = i2c_xfer(pI2c,(address << 1) | 1);
  if (r & 1) {
    i2c_stop(pI2c);
    return;
  }
  for (i = 0;i < length_write;i++) {
    r = i2c_xfer(pI2c,(pData_write[i]<<1) | 1);
    if (r & 1) {
      i2c_stop(pI2c);
      return;
    }
  }
  i2c_stop(pI2c);
}

void i2c_read_extended(i2c_config_t *pI2c, uint8_t address,
		       uint8_t *pData_write, int length_write,
		       uint8_t *pData_read, int length_read)
{
  int i;
  uint16_t r;

  i2c_start(pI2c);
  r = i2c_xfer(pI2c,(address << 1) | 1);
  if (r & 1) {
    i2c_stop(pI2c);
    return;
  }
  for (i = 0;i < length_write;i++) {
    r = i2c_xfer(pI2c,(pData_write[i]<<1) | 1);
  }
  i2c_start(pI2c);
  r = i2c_xfer(pI2c,(address << 1) | 3);
  if (r & 1) {
    i2c_stop(pI2c);
    return;
  }
  for (i = 0;i < length_read;i++) {
    if (i == length_read-1) {
      r = i2c_xfer(pI2c,0x1FF);
    } else {
      r = i2c_xfer(pI2c,0x1FE);
    }
    pData_read[i] = r >> 1;
  }
  i2c_stop(pI2c);
}

void hw_i2c_init(i2c_config_t *pI2c)
{
  LL_GPIO_InitTypeDef         GPIO_InitStructure;

  if ((pI2c->sclGpio == GPIOA) ||
      (pI2c->sdaGpio == GPIOA)) {
        
    __HAL_RCC_GPIOA_CLK_ENABLE();
  }
  if ((pI2c->sclGpio == GPIOB) ||
      (pI2c->sdaGpio == GPIOB)) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  }
  if ((pI2c->sclGpio == GPIOC) ||
      (pI2c->sdaGpio == GPIOC)) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  }
#ifdef GPIOF  
  if ((pI2c->sclGpio == GPIOF) ||
      (pI2c->sdaGpio == GPIOF)) {
    __HAL_RCC_GPIOF_CLK_ENABLE();
  }
#endif
  // Configure SCL in output mode
  GPIO_InitStructure.Pin = pI2c->sclPin;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(pI2c->sclGpio, &GPIO_InitStructure);

  // Configure SCL in output mode
  GPIO_InitStructure.Pin = pI2c->sdaPin;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(pI2c->sdaGpio, &GPIO_InitStructure);
}


void wait_us(int32_t n)
{
  int32_t start = TIM5->CNT;

  while ((TIM5->CNT - start) < n) {
  }
}

