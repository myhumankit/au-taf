#ifndef __BASE_H
#define __BASE_H

#include <stdint.h>
#include "stm32f4xx_ll_gpio.h"

/*
 * General part
 */
#define SIZEOF(arr) (sizeof(arr)/sizeof(arr[0]))

#define STR(a)  a

#define GPIO_AF_USART1 LL_GPIO_AF_7
#define GPIO_AF_USART2 LL_GPIO_AF_7
#define GPIO_AF_USART6 LL_GPIO_AF_8

#define gpio_of(n)      GPIO##n
#define GPIO_OF(n)      STR(gpio_of(n))

#define gpio_pin_of(n) LL_GPIO_PIN_##n
#define GPIO_PIN_OF(n) STR(gpio_pin_of(n))

#define GPIO_PIN(p,b)      GPIO_OF(p),GPIO_PIN_OF(b)

#define rcc_gpio_of(n)  RCC_AHB1Periph_GPIO##n
#define RCC_GPIO_OF(n)  STR(rcc_gpio_of(n))


typedef struct {
  GPIO_TypeDef  *Gpio;
  uint16_t      Pin;
} gpio_t;

#define GPIO_CONFIG(a,b) GPIO_PIN(a,b)


#define image_of(p) &outputImage##p
#define IMAGE(p) STR(image_of(p))

typedef struct {
  GPIO_TypeDef  *pGpio;
  uint16_t      pin;
  TIM_TypeDef   *pTim;
  uint16_t      channel;
  uint16_t      af;
  uint16_t      flag;
  uint16_t      polarity;
} edge_input_t;

typedef struct {
  GPIO_TypeDef  *pGpio;
  uint16_t      pin;
  uint16_t      *pImage;
  bool          bSet;
} output_t;

#define image_of(p) &outputImage##p
#define IMAGE(p) STR(image_of(p))
#define GPIO_OUTPUT_PIN(p,b)      GPIO_OF(p),GPIO_PIN_OF(b), IMAGE(p)


#define uart_of(n)      USART##n
#define UART_OF(n)      STR(uart_of(n))

#define uart_irq_of(n)      USART##n##_IRQn
#define UART_IRQ_OF(n)      STR(uart_irq_of(n))

#define uart_af_of(n)       GPIO_AF_USART##n
#define UART_AF_OF(n)       STR(uart_af_of(n))


#define uart_handler_of(n)      USART##n##_IRQHandler
#define UART_HANDLER_OF(n)      STR(uart_handler_of(n))

#define uart_clock_cmd_of(n)  UART_CLOCK_CMD##n
#define UART_CLOCK_CMD_OF(n)  STR(uart_clock_cmd_of(n))
#define UART_CLOCK_CMD(n,c)   UART_CLOCK_CMD_OF(n)(RCC_UART_OF(n),c)

#define rcc_uart_of(n)  RCC_UART##n
#define RCC_UART_OF(n)    STR(rcc_uart_of(n))


#define timer_of(n)    TIM##n
#define TIMER_OF(n)    STR(timer_of(n))

#define timer_channel_of(c) TIM_Channel_##c
#define TIMER_CHANNEL_OF(c) STR(timer_channel_of(c))

typedef struct {
  USART_TypeDef *uart;
  uint16_t      uartIrq;
  uint32_t      af;
  uint32_t      speed;
  GPIO_TypeDef  *txGpio;
  uint16_t      txPin;
  GPIO_TypeDef  *rxGpio;
  uint16_t      rxPin;
} uart_t;


#define UART_CONFIG(u) UART_OF(u),UART_IRQ_OF(u),UART_AF_OF(u)


int16_t ctoi( uint8_t c );
int16_t htoi(char *p);
uint32_t ltoi(char *p,int base);

void fmt_hexa8(uint8_t *pData, uint8_t value);
void fmt_hexa16(uint8_t *pData, uint16_t value);
void fmt_hexa32(uint8_t *pData, uint32_t value);
void put_c(char c);
void put_s(char *text);
void put_hexa8(uint32_t v);
void put_hexa32(uint32_t v);

#endif
