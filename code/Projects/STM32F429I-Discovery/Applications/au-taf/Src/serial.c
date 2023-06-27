#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
//#include "stm32f4xx_conf.h"
#include "main.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_dma.h"

#include "base.h"
#include "serial.h"

#define DYNAMIC_ALLOCATION 0
#define BUFFER_SIZE      64
#define U32_BUFFER_SIZE       256

#define RS485_ENABLE_ON        0x01
#define RS485_ENABLE_OFF       0x00

#include "buffer.h"

BUFFER_MANAGEMENT_DEF(1024,uint8_t,Serial)

#include "blocks.h"



Serial_t SerialCmdRxBuffer;
Serial_t SerialCmdTxBuffer;

uart_t uarts[] =
  {
   { UART_CONFIG(1), 115200, GPIO_PIN(A, 9), GPIO_PIN(A, 10)} // Command interpreter
  };

#define NB_UARTS SIZEOF(uarts)

void serialInit()
{
  int i;
  LL_GPIO_InitTypeDef       GPIO_InitStructure;
  LL_USART_InitTypeDef      USART_InitStructure;
  LL_USART_ClockInitTypeDef USART_ClockInitStructure;
  LL_DMA_InitTypeDef        DMA_InitStructure;

  SerialBufferInit( &SerialCmdRxBuffer );
  SerialBufferInit( &SerialCmdTxBuffer );

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  LL_DMA_DisableStream( DMA2, LL_DMA_STREAM_2 );
  DMA_InitStructure.Channel = LL_DMA_CHANNEL_4;
  DMA_InitStructure.PeriphOrM2MSrcAddress  = (uint32_t)&USART1->DR;
  DMA_InitStructure.MemoryOrM2MDstAddress  = (uint32_t)&SerialCmdRxBuffer.pData[0];
  DMA_InitStructure.Direction              = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  DMA_InitStructure.Mode                   = LL_DMA_MODE_CIRCULAR;
  DMA_InitStructure.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStructure.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStructure.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
  DMA_InitStructure.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
  DMA_InitStructure.NbData                 = sizeof(SerialCmdRxBuffer.pData);;
  DMA_InitStructure.Priority               = LL_DMA_PRIORITY_LOW;
  DMA_InitStructure.FIFOMode               = LL_DMA_FIFOMODE_DISABLE;
  DMA_InitStructure.FIFOThreshold          = LL_DMA_FIFOTHRESHOLD_1_4;
  DMA_InitStructure.MemBurst               = LL_DMA_MBURST_SINGLE;
  DMA_InitStructure.PeriphBurst            = LL_DMA_PBURST_SINGLE;

  LL_DMA_Init( DMA2, LL_DMA_STREAM_2, &DMA_InitStructure );
  LL_DMA_EnableStream( DMA2, LL_DMA_STREAM_2);
    
  for (i = 0;i < NB_UARTS;i++) {
    // Configure in alternate input mode
    GPIO_InitStructure.Pin = uarts[i].rxPin;
    GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStructure.Alternate = uarts[i].af;
    LL_GPIO_Init(uarts[i].rxGpio, &GPIO_InitStructure);

    // Configure PA2 in alternate output pushpull mode
    GPIO_InitStructure.Pin = uarts[i].txPin;
    GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStructure.Alternate = uarts[i].af;
    LL_GPIO_Init(uarts[i].txGpio, &GPIO_InitStructure);
    
    // Conf

    LL_USART_ClockStructInit( &USART_ClockInitStructure );
    LL_USART_ClockInit( uarts[i].uart, &USART_ClockInitStructure );

    USART_InitStructure.BaudRate = uarts[i].speed;
    USART_InitStructure.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStructure.StopBits = LL_USART_STOPBITS_1;
    USART_InitStructure.Parity = LL_USART_PARITY_NONE;
    USART_InitStructure.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStructure.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStructure.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(uarts[i].uart, &USART_InitStructure);

    // Config DMA for RX
    LL_USART_EnableDMAReq_RX(uarts[i].uart);
    // Enable
    LL_USART_Enable(uarts[i].uart);
  }
}


const gpio_t spi_ios[] =
  {
   { GPIO_CONFIG(F, 7) }, // SPI_SCK
   { GPIO_CONFIG(F, 8) }, // SPI_MISO
   { GPIO_CONFIG(F, 9) }, // SPI_MOSI
 };
const gpio_t simple_ios[] =
  {
   { GPIO_CONFIG(C, 1) }, // NCSS
   { GPIO_CONFIG(C, 3) }, // Mux Bit 0 
   { GPIO_CONFIG(A, 5) }, // Mux Bit 1
 };

enum {
      SIMPLE_IOS_NCSS=0,
      SIMPLE_IOS_MUX_BIT_0,
      SIMPLE_IOS_MUX_BIT_1
};

void spiInit()
{
  int i;
  LL_GPIO_InitTypeDef       GPIO_InitStructure;
  LL_SPI_InitTypeDef        SPI_InitStructure;
  
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  for (i = 0;i < SIZEOF(spi_ios);i++) {
    // Configure led IO in output pushpull mode
    GPIO_InitStructure.Pin = spi_ios[i].Pin;
    GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStructure.Alternate = LL_GPIO_AF_5;
    LL_GPIO_Init(spi_ios[i].Gpio, &GPIO_InitStructure);
  }    

  
  for (i = 0;i < SIZEOF(simple_ios);i++) {
    // Configure led IO in output pushpull mode
    GPIO_InitStructure.Pin = simple_ios[i].Pin;
    GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(simple_ios[i].Gpio, &GPIO_InitStructure);
  }
  
  // High  level
  LL_GPIO_SetOutputPin(simple_ios[SIMPLE_IOS_NCSS].Gpio, simple_ios[SIMPLE_IOS_NCSS].Pin);
  // Mux 00
  LL_GPIO_ResetOutputPin(simple_ios[SIMPLE_IOS_MUX_BIT_1].Gpio, simple_ios[SIMPLE_IOS_MUX_BIT_1].Pin);
  LL_GPIO_ResetOutputPin(simple_ios[SIMPLE_IOS_MUX_BIT_0].Gpio, simple_ios[SIMPLE_IOS_MUX_BIT_0].Pin);
  
  
  LL_SPI_StructInit(&SPI_InitStructure);
  
  SPI_InitStructure.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStructure.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStructure.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStructure.ClockPolarity = LL_SPI_POLARITY_HIGH;
  SPI_InitStructure.ClockPhase = LL_SPI_PHASE_2EDGE;
  SPI_InitStructure.NSS = LL_SPI_NSS_HARD_INPUT;
  SPI_InitStructure.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV128;
  SPI_InitStructure.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStructure.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;

  LL_SPI_Init(SPI5,&SPI_InitStructure);

}

void mux_set_switch(int n)
{
  // Mux MSB
  if (n & 2) {  
    LL_GPIO_SetOutputPin(simple_ios[SIMPLE_IOS_MUX_BIT_1].Gpio, simple_ios[SIMPLE_IOS_MUX_BIT_1].Pin);
  } else {
    LL_GPIO_ResetOutputPin(simple_ios[SIMPLE_IOS_MUX_BIT_1].Gpio, simple_ios[SIMPLE_IOS_MUX_BIT_1].Pin);
  }

  if (n & 1) {
    LL_GPIO_SetOutputPin(simple_ios[SIMPLE_IOS_MUX_BIT_0].Gpio, simple_ios[SIMPLE_IOS_MUX_BIT_0].Pin);
  } else {
    LL_GPIO_ResetOutputPin(simple_ios[SIMPLE_IOS_MUX_BIT_0].Gpio, simple_ios[SIMPLE_IOS_MUX_BIT_0].Pin);
  }
}


void spi_send(uint8_t *data, int n)
{
  int i;

  if (n < 1) {
    return;
  }
  
  LL_SPI_Enable(SPI5);

  LL_GPIO_ResetOutputPin(simple_ios[SIMPLE_IOS_NCSS].Gpio, simple_ios[SIMPLE_IOS_NCSS].Pin);

  for (i = 0;i < n;i++) {
    while (LL_SPI_IsActiveFlag_BSY(SPI5) != 0)
      ;
    LL_SPI_TransmitData8(SPI5,data[i]);
    
  }

  while (LL_SPI_IsActiveFlag_BSY(SPI5) != 0)
    ;
  
  LL_GPIO_SetOutputPin(simple_ios[SIMPLE_IOS_NCSS].Gpio, simple_ios[SIMPLE_IOS_NCSS].Pin);
  
  LL_SPI_Disable(SPI5);
}

void l3gd20_set_three_state(void)
{
  uint8_t data[5];
  
  // 0x60 10 00 30 00 : INT1 OK, INT2 Low
  data[0] = 0x40 | 0x20;
  data[1] = 0x08; // Powerdown
  data[2] = 0x00;
  data[3] = 0x10;
  data[4] = 0x00;
  
  spi_send(data,5);
  
}

#define IR_TIMER_MHZ 1

void timerInit(void *data, int n)
{
  LL_TIM_InitTypeDef    TIM_InitStructure;
  LL_TIM_IC_InitTypeDef IC_InitStructure;
  LL_TIM_OC_InitTypeDef OC_InitStructure;
  LL_DMA_InitTypeDef    DMA_InitStructure;
  LL_GPIO_InitTypeDef   GPIO_InitStructure;

  
  __HAL_RCC_TIM5_CLK_ENABLE();
  __HAL_RCC_TIM10_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  // Configure compare in output pushpull mode
  GPIO_InitStructure.Pin = LL_GPIO_PIN_6;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStructure.Alternate = LL_GPIO_AF_3;
  LL_GPIO_Init(GPIOF, &GPIO_InitStructure);
  
  // Configure compare in output pushpull mode
  GPIO_InitStructure.Pin = LL_GPIO_PIN_2;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStructure.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  // Configure capture in output pushpull mode
  GPIO_InitStructure.Pin = LL_GPIO_PIN_1;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStructure.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  LL_TIM_StructInit(&TIM_InitStructure);

  TIM_InitStructure.Prescaler = 90-1;
  TIM_InitStructure.CounterMode = LL_TIM_COUNTERMODE_UP;
  
  LL_TIM_Init(TIM5, &TIM_InitStructure);

  LL_TIM_OC_StructInit(&OC_InitStructure);
  
  OC_InitStructure.OCMode = LL_TIM_OCMODE_TOGGLE;
  OC_InitStructure.OCState = LL_TIM_OCSTATE_DISABLE;
  OC_InitStructure.OCNState  =  LL_TIM_OCSTATE_DISABLE;
  OC_InitStructure.OCPolarity = LL_TIM_OCPOLARITY_LOW;

  OC_InitStructure.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
  

  LL_TIM_OC_Init(TIM5, LL_TIM_CHANNEL_CH3, &OC_InitStructure);
  LL_TIM_CC_DisableChannel(TIM5,LL_TIM_CHANNEL_CH3);

  LL_TIM_IC_StructInit(&IC_InitStructure);

  IC_InitStructure.ICPolarity = LL_TIM_IC_POLARITY_BOTHEDGE;
  IC_InitStructure.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
  IC_InitStructure.ICPrescaler = LL_TIM_ICPSC_DIV1;
  IC_InitStructure.ICFilter = LL_TIM_IC_FILTER_FDIV1;

  LL_TIM_IC_Init(TIM5, LL_TIM_CHANNEL_CH2, &IC_InitStructure);
  

  LL_DMA_DisableStream( DMA1, LL_DMA_STREAM_4 );
  DMA_InitStructure.Channel = LL_DMA_CHANNEL_6;
  DMA_InitStructure.PeriphOrM2MSrcAddress  = (uint32_t)&TIM5->CCR2;
  DMA_InitStructure.MemoryOrM2MDstAddress  = (uint32_t)data;
  DMA_InitStructure.Direction              = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  DMA_InitStructure.Mode                   = LL_DMA_MODE_CIRCULAR;
  DMA_InitStructure.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStructure.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStructure.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  DMA_InitStructure.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  DMA_InitStructure.NbData                 = n;
  DMA_InitStructure.Priority               = LL_DMA_PRIORITY_LOW;
  DMA_InitStructure.FIFOMode               = LL_DMA_FIFOMODE_DISABLE;
  DMA_InitStructure.FIFOThreshold          = LL_DMA_FIFOTHRESHOLD_1_4;
  DMA_InitStructure.MemBurst               = LL_DMA_MBURST_SINGLE;
  DMA_InitStructure.PeriphBurst            = LL_DMA_PBURST_SINGLE;

  LL_DMA_Init( DMA1, LL_DMA_STREAM_4, &DMA_InitStructure );
  LL_DMA_EnableStream( DMA1, LL_DMA_STREAM_4);


  LL_TIM_EnableCounter(TIM5);
  
  LL_TIM_EnableDMAReq_CC2(TIM5);

  LL_TIM_CC_EnableChannel(TIM5,LL_TIM_CHANNEL_CH2);
}

void set_modulation(int rate)
{
  LL_TIM_InitTypeDef    TIM_InitStructure;
  LL_TIM_OC_InitTypeDef OC_InitStructure;
  
  LL_TIM_DisableCounter(TIM10);

  LL_TIM_StructInit(&TIM_InitStructure);

  TIM_InitStructure.Prescaler = 1-1;
  TIM_InitStructure.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStructure.Autoreload = rate;
  
  LL_TIM_Init(TIM10, &TIM_InitStructure);

  LL_TIM_OC_StructInit(&OC_InitStructure);
  
  OC_InitStructure.OCMode = LL_TIM_OCMODE_PWM1;        /*!< Specifies the output mode.
                               This parameter can be a value of @ref TIM_LL_EC_OCMODE.

                               This feature can be modified afterwards using unitary function
                               @ref LL_TIM_OC_SetMode().*/

  OC_InitStructure.OCState = LL_TIM_OCSTATE_DISABLE;       /*!< Specifies the TIM Output Compare state.
                               This parameter can be a value of @ref TIM_LL_EC_OCSTATE.

                               This feature can be modified afterwards using unitary functions
                               @ref LL_TIM_CC_EnableChannel() or @ref LL_TIM_CC_DisableChannel().*/

  OC_InitStructure.OCNState  =  LL_TIM_OCSTATE_DISABLE;
  OC_InitStructure.CompareValue = rate - (rate >>2);  /*!< Specifies the Compare value to be loaded into the Capture Compare Register.
                               This parameter can be a number between Min_Data=0x0000 and Max_Data=0xFFFF.

                               This feature can be modified afterwards using unitary function
                               LL_TIM_OC_SetCompareCHx (x=1..6).*/

  OC_InitStructure.OCPolarity = LL_TIM_OCPOLARITY_LOW;    /*!< Specifies the output polarity.
                               This parameter can be a value of @ref TIM_LL_EC_OCPOLARITY.

                               This feature can be modified afterwards using unitary function
                               @ref LL_TIM_OC_SetPolarity().*/

  OC_InitStructure.OCIdleState = LL_TIM_OCIDLESTATE_LOW;   /*!< Specifies the TIM Output Compare pin state during Idle state.
                               This parameter can be a value of @ref TIM_LL_EC_OCIDLESTATE.

                               This feature can be modified afterwards using unitary function
                               @ref LL_TIM_OC_SetIdleState().*/


  LL_TIM_CC_DisableChannel(TIM10,LL_TIM_CHANNEL_CH1);
  LL_TIM_OC_Init(TIM10, LL_TIM_CHANNEL_CH1, &OC_InitStructure);
  LL_TIM_CC_EnableChannel(TIM10,LL_TIM_CHANNEL_CH1);
  LL_TIM_EnableCounter(TIM10);
}

#define TOTAL 646
#define FIRST 200

void timer_send(uint32_t time, uint32_t *pData, int n)
{
  LL_DMA_InitTypeDef    DMA_InitStructure;
  LL_GPIO_InitTypeDef   GPIO_InitStructure;
  
  // Configure compare in output pushpull mode
  GPIO_InitStructure.Pin = LL_GPIO_PIN_2;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStructure.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  LL_TIM_CC_DisableChannel(TIM5,LL_TIM_CHANNEL_CH3);
  
  TIM5->CCR3 = time;
  LL_DMA_DisableStream( DMA1, LL_DMA_STREAM_0 );
  DMA_InitStructure.Channel = LL_DMA_CHANNEL_6;
  DMA_InitStructure.PeriphOrM2MSrcAddress  = (uint32_t)&TIM5->CCR3;
  DMA_InitStructure.MemoryOrM2MDstAddress  = (uint32_t)pData;
  DMA_InitStructure.Direction              = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  DMA_InitStructure.Mode                   = LL_DMA_MODE_NORMAL;
  DMA_InitStructure.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStructure.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStructure.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  DMA_InitStructure.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  DMA_InitStructure.NbData                 = n;
  DMA_InitStructure.Priority               = LL_DMA_PRIORITY_LOW;
  DMA_InitStructure.FIFOMode               = LL_DMA_FIFOMODE_DISABLE;
  DMA_InitStructure.FIFOThreshold          = LL_DMA_FIFOTHRESHOLD_1_4;
  DMA_InitStructure.MemBurst               = LL_DMA_MBURST_SINGLE;
  DMA_InitStructure.PeriphBurst            = LL_DMA_PBURST_SINGLE;

  LL_DMA_Init( DMA1, LL_DMA_STREAM_0, &DMA_InitStructure );
  LL_DMA_EnableStream( DMA1, LL_DMA_STREAM_0);

  LL_TIM_EnableDMAReq_CC3(TIM5);

  LL_TIM_CC_EnableChannel(TIM5,LL_TIM_CHANNEL_CH3);
  
}


#define TOTAL 646
#define FIRST 200

uint32_t generate_perel(uint32_t time, uint32_t code, uint32_t *pData, int max)
{
  int n = 0;
  int i;
  int j;

  if (max < 250) {
    return 0;
  }
  
  time += 100;
  pData[n++] = time;
  for (j = 0;j < 5;j++) {
    for (i = 0;i < 24;i++) {
      if (code & (0x800000 >>i)) {
	time += (TOTAL-FIRST);
	pData[n++] = time;
	time += FIRST;
	pData[n++] = time;
      } else {
	time += FIRST;
	pData[n++] = time;
	time += (TOTAL-FIRST);
	pData[n++] = time;
      }
    }
    time += FIRST;
    pData[n++] = time;
    time += (TOTAL-FIRST);
    pData[n++] = time;
    if (j != 4) {
      time += 5000;
    }
    pData[n-1] = time;
  }

  return n-1;
}


#define IR_TIMER_PHILIPS_HEADER        (2800*IR_TIMER_MHZ)
#define IR_TIMER_PHILIPS_HALF_TIME     (442*IR_TIMER_MHZ)

#define PHILIPS_INITIAL_TOGGLE 0x10000
#define PHILIPS_TOGGLE         0x30000

uint32_t philips_toggle = PHILIPS_INITIAL_TOGGLE;

uint32_t generate_philips(uint32_t time, uint32_t code, uint32_t *pData, int max)
{
  int n = 0;
  uint32_t mask;
  int16_t  i;
  uint8_t level = 0;


  if (max < 46+3) {
    return 0;
  }
  time += 100;
  pData[n++] = time;

  time += IR_TIMER_PHILIPS_HEADER;

  for (i = 22;i >= 0;i--) {
    mask = 1 << i;
    if (mask & 0x430000) {
      if (((code >> i) & 1) == level) {
      } else {
	pData[n++] = time;
	level = 1 - level;
      }
      time += 2 * IR_TIMER_PHILIPS_HALF_TIME;
    } else {
      if (((code >> i) & 1) == level) {
	pData[n++] = time;
	time += IR_TIMER_PHILIPS_HALF_TIME;
	pData[n++] = time;
      } else {
	time += IR_TIMER_PHILIPS_HALF_TIME;
	pData[n++] = time;
	level = 1 - level;
      }
      time += IR_TIMER_PHILIPS_HALF_TIME;
    }	
  }
  
  pData[n++] = time;
  time += IR_TIMER_PHILIPS_HALF_TIME;
  pData[n++] = time;

  if (level != 0) {
    time += IR_TIMER_PHILIPS_HALF_TIME;
    pData[n++] = time;
  }
  return n-1;
}


#define IR_TIMER_AKAI_TAU           (565*IR_TIMER_MHZ)
#define IR_TIMER_AKAI_HEADER_1      (16*IR_TIMER_AKAI_TAU)
#define IR_TIMER_AKAI_HEADER_2      (8*IR_TIMER_AKAI_TAU)
#define IR_TIMER_AKAI_INITIAL_PULSE (1*IR_TIMER_AKAI_TAU)
#define IR_TIMER_AKAI_ZERO          (1*IR_TIMER_AKAI_TAU)
#define IR_TIMER_AKAI_ONE           (3*IR_TIMER_AKAI_TAU)


uint32_t generate_akai(uint32_t time, uint32_t code, uint32_t *pData, int max)
{
  int n = 0;
  uint32_t full;
  uint32_t mask = 0x1;


  if (max < 2+64+2) {
    return 0;
  }
  time += 100;
  pData[n++] = time;
  
  time += IR_TIMER_AKAI_HEADER_1;
  pData[n++] = time;
  time += IR_TIMER_AKAI_HEADER_2;
  pData[n++] = time;

  full = code;
  full = ((full & 0xFF00) >> 8) |
    ((full & 0xFF00) ^ 0xFF00) |
    ((full & 0xFF) << 16) |
    (((full & 0xFF) ^ 0xFF) << 24);

  
  while (mask != 0) {
    time += IR_TIMER_AKAI_INITIAL_PULSE;
    pData[n++] = time;
    if ( mask & full) {
      time += IR_TIMER_AKAI_ONE;
    } else {
      time += IR_TIMER_AKAI_ZERO;
    }
    pData[n++] = time;

    mask <<= 1;
  }
  time += IR_TIMER_AKAI_INITIAL_PULSE;
  pData[n++] = time;
  time += IR_TIMER_AKAI_INITIAL_PULSE;
  pData[n++] = time;
  return n-1;
}


// Case we use infra red 
#define IR_TIMER_RC5_HEADER        888*IR_TIMER_MHZ
#define IR_TIMER_RC5_HALF_TIME     888*IR_TIMER_MHZ

uint32_t generate_rc5(uint32_t time, uint32_t code, uint32_t *pData, int max)
{
  int n = 0;
  int16_t  i;
  int16_t  ibis;
  uint8_t level = 1;


  if (max < 24+2) {
    return 0;
  }
  time += 100;
  pData[n++] = time;
  
  time += IR_TIMER_RC5_HEADER;

  for (i = 12;i >= 0;i--) {
    if (i > 6) {
      ibis = i + 1;
    } else {
      ibis = i;
    }
    if (((code >> ibis) & 1) == level) {
      pData[n++] = time;
      time += IR_TIMER_RC5_HALF_TIME;
      pData[n++] = time;
    } else {
      time += IR_TIMER_RC5_HALF_TIME;
      pData[n++] = time;
      level = 1 - level;
    }
    time += IR_TIMER_RC5_HALF_TIME;
  }
  
  pData[n++] = time;
  if (level == 1) {
    time += IR_TIMER_RC5_HALF_TIME;
    pData[n++] = time;
  }
  return n-1;
}

#define IR_TIMER_AAA_HALF_TIME     848*IR_TIMER_MHZ

uint32_t generate_aaa(uint32_t time, uint32_t code, uint32_t *pData, int max)
{
  int n = 0;
  int16_t  i;
  int16_t  ibis;
  uint8_t level = 0;
  static uint32_t toggle = 0;

  if (max < 24+2) {
    return 0;
  }
  time += 100;
  pData[n++] = time;
  

  time += IR_TIMER_AAA_HALF_TIME;

  code ^= toggle;
  for (i = 12;i >= 0;i--) {
    if (i > 6) {
      ibis = i + 1;
    } else {
      ibis = i;
    }
    if (((code >> ibis) & 1) == level) {
      time += IR_TIMER_AAA_HALF_TIME;
      pData[n++] = time;
      level = 1 - level;
    } else {
      pData[n++] = time;
      time += IR_TIMER_AAA_HALF_TIME;
      pData[n++] = time;
    }
    time += IR_TIMER_AAA_HALF_TIME;
  }
  
  pData[n++] = time;
  if (level == 0) {
    time += IR_TIMER_AAA_HALF_TIME;
    pData[n++] = time;
  }

  toggle ^= 0x1000;
  return n-1;
}

