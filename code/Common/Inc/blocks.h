#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#if 0
#define UART_TX_NB   1
#define UART_RX_NB   1

#define UART_TX_GPIO    B
#define UART_TX_PIN     6
#define UART_RX_GPIO    B
#define UART_RX_PIN     7

#define STR(a)  a

#define GPIO_AF_TIM1   LL_GPIO_AF_1
#define GPIO_AF_TIM2   LL_GPIO_AF_1

#define GPIO_AF_TIM3   LL_GPIO_AF_2
#define GPIO_AF_TIM4   LL_GPIO_AF_2
#define GPIO_AF_TIM5   LL_GPIO_AF_2

#define GPIO_AF_TIM9   LL_GPIO_AF_3
#define GPIO_AF_TIM10  LL_GPIO_AF_3
#define GPIO_AF_TIM11  LL_GPIO_AF_3

#define GPIO_AF_I2C1   LL_GPIO_AF_4

#define GPIO_AF_SPI1   LL_GPIO_AF_5
#define GPIO_AF_I2S1   LL_GPIO_AF_5

#define GPIO_AF_SPI4   LL_GPIO_AF_5
#define GPIO_AF_I2S4   LL_GPIO_AF_6
#define GPIO_AF_SPI5   LL_GPIO_AF_6
#define GPIO_AF_I2S5   LL_GPIO_AF_6

#define GPIO_AF_USART1 LL_GPIO_AF_7
#define GPIO_AF_USART2 LL_GPIO_AF_7

#define GPIO_AF_USART6 LL_GPIO_AF_8

#define GPIO_AF_OTG1_FS   LL_GPIO_AF_10

#define GPIO_AF_SDIO   LL_GPIO_AF_12

#define uart_of(n)      USART##n
#define UART_OF(n)      STR(uart_of(n))
#define UART_TX         UART_OF(UART_TX_NB)
#define UART_RX         UART_OF(UART_RX_NB)

#define RCC_UART1       RCC_APB2Periph_USART1
#define RCC_UART2       RCC_APB1Periph_USART2
#define RCC_UART6       RCC_APB2Periph_USART6

#define UART_CLOCK_CMD1 RCC_APB2PeriphClockCmd
#define UART_CLOCK_CMD2 RCC_APB1PeriphClockCmd
#define UART_CLOCK_CMD3 RCC_APB1PeriphClockCmd
#define UART_CLOCK_CMD4 RCC_APB1PeriphClockCmd
#define UART_CLOCK_CMD5 RCC_APB1PeriphClockCmd
#define UART_CLOCK_CMD6 RCC_APB2PeriphClockCmd
#define UART_CLOCK_CMD7 RCC_APB1PeriphClockCmd
#define UART_CLOCK_CMD8 RCC_APB1PeriphClockCmd

#define uart_handler_of(n)      USART##n##_IRQHandler
#define UART_HANDLER_OF(n)      STR(uart_handler_of(n))

#define uart_irq_of(n)      USART##n##_IRQn
#define UART_IRQ_OF(n)      STR(uart_irq_of(n))

#define rcc_uart_of(n)  RCC_UART##n
#define RCC_UART_OF(n)    STR(rcc_uart_of(n))
//#define RCC_UART_TX     RCC_UART_OF(UART_TX_NB)
//#define RCC_UART_RX     RCC_UART_OF(UART_RX_NB)

#define uart_clock_cmd_of(n)  UART_CLOCK_CMD##n
#define UART_CLOCK_CMD_OF(n)  STR(uart_clock_cmd_of(n))
#define UART_CLOCK_CMD(n,c)   UART_CLOCK_CMD_OF(n)(RCC_UART_OF(n),c)

#define gpio_af_of(n)   GPIO_AF_USART##n
#define GPIO_AF_OF(n)   STR(gpio_af_of(n))
#define GPIO_AF_TX      GPIO_AF_OF(UART_TX_NB)
#define GPIO_AF_RX      GPIO_AF_OF(UART_RX_NB)

#define rcc_gpio_of(n)  RCC_AHB1Periph_GPIO##n
#define RCC_GPIO_OF(n)  STR(rcc_gpio_of(n))
#define RCC_GPIO_TX     RCC_GPIO_OF(UART_TX_GPIO)
#define RCC_GPIO_RX     RCC_GPIO_OF(UART_RX_GPIO)

#define gpio_of(n)      GPIO##n
#define GPIO_OF(n)      STR(gpio_of(n))
#define GPIO_TX         GPIO_OF(UART_TX_GPIO)
#define GPIO_RX         GPIO_OF(UART_RX_GPIO)

// Bit mask (0x0001-0x8000)
#define gpio_pin_of(n) LL_GPIO_PIN_##n
#define GPIO_PIN_OF(n) STR(gpio_pin_of(n))
#define GPIO_PIN_TX    GPIO_PIN_OF(UART_TX_PIN)
#define GPIO_PIN_RX    GPIO_PIN_OF(UART_RX_PIN)

#define timer_of(n)    TIM##n
#define TIMER_OF(n)    STR(timer_of(n))
#define gpio_af_tim(f) GPIO_AF_TIM##f
#define GPIO_AF_TIM(t) STR(gpio_af_tim(t))

#define timer_channel_of(c) TIM_Channel_##c
#define TIMER_CHANNEL_OF(c) STR(timer_channel_of(c))

// Private function prototypes
void Delay(volatile uint32_t nCount);


#ifdef BLOCKS_HTOI
uint8_t htoi( uint8_t c )
{
  if (( c >= '0') && ( c <= '9' ))
    return( c - '0' );

  if (( c >= 'A') && ( c <= 'F' ))
    return( c - 'A' + 10 );
  if (( c >= 'a') && ( c <= 'f' ))
    return( c - 'a' + 10 );

  return( 0 );
}
#endif
#endif

#ifdef BLOCKS_BUFFER

#ifndef BUFFER_SIZE
#define BUFFER_SIZE      512
#endif
#if 0
typedef struct 
{
#if DYNAMIC_ALLOCATION
    uint8_t     *pData;
#else
    uint8_t     pData[BUFFER_SIZE];
#endif
    uint16_t    ui16Length;
    uint16_t    ui16WriteIndex;
    uint16_t    ui16ReadIndex;
} Uint8Buffer;


typedef struct 
{
    uint32_t    pData[U32_BUFFER_SIZE];
    uint16_t    ui16Length;
    uint16_t    ui16WriteIndex;
    uint16_t    ui16ReadIndex;
} Uint32Buffer;

void Buffer_Init( Uint8Buffer *pBuffer, uint16_t ui16Length )
{
#if DYNAMIC_ALLOCATION
  if ( ui16Length > pBuffer->ui16Length ) {
    if ( NULL != pBuffer->pData ) {
      free( pBuffer->pData );
      pBuffer->pData = NULL;
    }
    pBuffer->pData = malloc( ui16Length );
  }
  pBuffer->ui16Length = ui16Length;
#else
  pBuffer->ui16Length = sizeof( pBuffer->pData ) / sizeof( pBuffer->pData[0]);
#endif
  pBuffer->ui16WriteIndex = 0;
  pBuffer->ui16ReadIndex = 0;
}

void Buffer_Push( Uint8Buffer *pBuffer, uint8_t ui8Data )
{
  if (( NULL == pBuffer) ||
      ( NULL == pBuffer->pData )) {
    return;
  }

  pBuffer->pData[pBuffer->ui16WriteIndex] = ui8Data;
  pBuffer->ui16WriteIndex++;
  if ( pBuffer->ui16WriteIndex >= pBuffer->ui16Length ) {
    pBuffer->ui16WriteIndex = 0;
  }
}

bool Buffer_IsEmpty( Uint8Buffer *pBuffer )
{
  return( pBuffer->ui16ReadIndex == pBuffer->ui16WriteIndex );
}

uint8_t Buffer_Pop( Uint8Buffer *pBuffer)
{
  uint8_t ui8Data;

  if (( NULL == pBuffer) ||
      ( NULL == pBuffer->pData )) {
    return( 0 );
  }

  ui8Data = pBuffer->pData[pBuffer->ui16ReadIndex];
  pBuffer->ui16ReadIndex++;
  if ( pBuffer->ui16ReadIndex >= pBuffer->ui16Length ) {
    pBuffer->ui16ReadIndex = 0;
  }
  return( ui8Data );
}
static void Buffer_PutHexa2( Uint8Buffer *pBuffer, uint8_t ui8Data )
{
  static uint8_t aui8Hexa[] = {
    '0','1','2','3','4','5','6','7',
    '8','9','A','B','C','D','E','F'};
  Buffer_Push( pBuffer, aui8Hexa[(ui8Data >> 4) & 0x0F]);
  Buffer_Push( pBuffer, aui8Hexa[ui8Data & 0x0F]);
}

void Buffer_PutHexa4( Uint8Buffer *pBuffer, uint16_t ui16Data )
{
  Buffer_PutHexa2( pBuffer, ui16Data >> 8 );
  Buffer_PutHexa2( pBuffer, ui16Data );
}

void Buffer_PutHexa8( Uint8Buffer *pBuffer, uint32_t ui32Data )
{
  Buffer_PutHexa4( pBuffer, ui32Data >> 16 );
  Buffer_PutHexa4( pBuffer, ui32Data );
}

void Buffer_PutDeci8( Uint8Buffer *pBuffer, uint32_t ui32Data, uint8_t ui8Width )
{
  uint8_t aui8Buffer[10];
  uint8_t ui8Index = 0;

  do {
    aui8Buffer[ui8Index++] = ui32Data % 10;
    ui32Data = ui32Data / 10;
  } while ( ui32Data != 0 );

  while ( ui8Index < ui8Width ) {
    Buffer_Push( pBuffer, ' ' );
    ui8Width--;
  }

  do {
    ui8Index--;
    Buffer_Push( pBuffer, '0' + aui8Buffer[ui8Index] );
  } while ( ui8Index != 0 );
}

void Uint32Buffer_Init( Uint32Buffer *pBuffer, uint16_t ui16Length )
{
  pBuffer->ui16Length = sizeof( pBuffer->pData ) / sizeof( pBuffer->pData[0]);
  pBuffer->ui16WriteIndex = 0;
  pBuffer->ui16ReadIndex = 0;
}

void Uint32Buffer_Push( Uint32Buffer *pBuffer, uint32_t ui32Data )
{
  if ( NULL == pBuffer) {
    return;
  }

  pBuffer->pData[pBuffer->ui16WriteIndex] = ui32Data;
  pBuffer->ui16WriteIndex++;
  if ( pBuffer->ui16WriteIndex >= pBuffer->ui16Length ) {
    pBuffer->ui16WriteIndex = 0;
  }
}

bool Uint32Buffer_IsEmpty( Uint32Buffer *pBuffer )
{
  return( pBuffer->ui16ReadIndex == pBuffer->ui16WriteIndex );
}

uint32_t Uint32Buffer_Pop( Uint32Buffer *pBuffer)
{
  uint32_t ui32Data;

  if (( NULL == pBuffer) ||
      ( NULL == pBuffer->pData )) {
    return( 0 );
  }

  ui32Data = pBuffer->pData[pBuffer->ui16ReadIndex];
  pBuffer->ui16ReadIndex++;
  if ( pBuffer->ui16ReadIndex >= pBuffer->ui16Length ) {
    pBuffer->ui16ReadIndex = 0;
  }
  return( ui32Data );
}
#endif

void Rx_Loop( Uint8Buffer *pBuffer )
{
  uint8_t ui8Data = 0;

  if ( USART_GetFlagStatus( UART_RX, USART_FLAG_RXNE ) ) {
    ui8Data = USART_ReceiveData( UART_RX );
    Buffer_Push( pBuffer, ui8Data );
  }
}

void Tx_Loop( Uint8Buffer *pBuffer )
{
  uint8_t ui8Data = 0;

  if ( !Buffer_IsEmpty( pBuffer ) &&
       ( USART_GetFlagStatus( UART_TX, USART_FLAG_TXE ) ) ) {
    ui8Data = Buffer_Pop( pBuffer );
    USART_SendData( UART_TX, ui8Data );
  } else if  ( USART_GetFlagStatus( UART_TX, USART_FLAG_TXE ) ) {
    //    Buffer_PutHexa8( &TxBuffer, *(uint32_t*)0);
    //    USART_SendData( UART_TX, '=' );
  }
}
#else
#endif

#if 0
#define tim_flag(c)        TIM_FLAG_CC##c
#define TIM_FLAG(c)        STR(tim_flag(c))
#define GPIO_PIN(p,b)      GPIO_OF(p),GPIO_PIN_OF(b)
#define TIMER_CHANNEL_AF(t,c) TIMER_OF(t),TIMER_CHANNEL_OF(c),GPIO_AF_TIM(t),TIM_FLAG(c)

typedef struct {
  GPIO_TypeDef  *pGpio;
  uint16_t      pinSource;
  uint16_t      pin;
  TIM_TypeDef   *pTim;
  uint16_t      channel;
  uint16_t      af;
  uint16_t      flag;
  uint16_t      polarity;
} edge_input_t;


typedef struct {
  GPIO_TypeDef  *pGpio;
  uint16_t      pinSource;
  uint16_t      pin;
  uint16_t      *pImage;
  bool          bSet;
} output_t;
#endif
#if 0
#define image_of(p) &outputImage##p
#define IMAGE(p) STR(image_of(p))
#define GPIO_OUTPUT_PIN(p,b)      GPIO_OF(p),GPIO_PIN_SOURCE_OF(b), GPIO_PIN_OF(b), IMAGE(p)


#define COMBINE(f,v,shift) {				\
    uint16_t data       = 0;				\
    uint16_t mask       = 0;				\
    mask = ((v >> 8) & 0xFF) << shift;			\
    data = (v & 0xFF) << shift;				\
    (f) = ((f) & ~mask) | (data & mask);		\
  }


#define OUTPUTS_NB (sizeof(outputs)/sizeof(outputs[0]))

typedef struct rs485_context_t rs485_context_t;

struct rs485_context_t {
  uint8_t txdataBuffer[256];
  uint16_t ui16SendingIndex;
  uint16_t dataIndex;
  bool bSending;
  bool bEnable;
};
#endif
#ifdef BLOCKS_RS485
void Rs485_Enable( uint16_t value );

void Rs485_Init(rs485_context_t *pContext)
{
  memset(pContext,0,sizeof(*pContext));
}

void Rs485_Start(rs485_context_t *pContext)
{
  if (0 != pContext->dataIndex) {
    pContext->ui16SendingIndex = 0;
    Rs485_Enable(RS485_ENABLE_ON );
    pContext->bSending = true;
    pContext->bEnable = true;
  }
}
void Rs485_Send(rs485_context_t *pContext, uint8_t *pData, uint16_t n)
{
  if (pContext->bEnable == false) {
    uint16_t i;
    pContext->dataIndex = n;

    for (i = 0;i < n;i++) {
      pContext->txdataBuffer[i] = pData[i];
    }
  }
}
void Rs485_Loop(rs485_context_t *pContext)
{
  if ( pContext->bSending ) {
    if ( LL_USART_IsActiveFlag_TXE( USART2 ) ) {
      uint8_t ui8Data;
      
      ui8Data = pContext->txdataBuffer[pContext->ui16SendingIndex++];
      LL_USART_TransmitData8( USART2, ui8Data );
      if ( pContext->ui16SendingIndex >= pContext->dataIndex ) {
	pContext->dataIndex = 0;
	pContext->bSending = false;
      }
    }
  } else if ( pContext->bEnable ) {
    if ( LL_USART_IsActiveFlag_TC( USART2 ) ) {
      Rs485_Enable(RS485_ENABLE_OFF );
      pContext->bEnable = false;
    }
  }
}

#endif

#define HANDLER(u,buf) void UART_HANDLER_OF(u)(void)	\
  {						\
    uint8_t  ui8Data  = 0;					\
    if ( LL_USART_IsActiveFlag_RXNE( UART_OF(u) ) )	\
      {									\
	ui8Data = LL_USART_ReceiveData8( UART_OF(u) );			\
	SerialBufferPush( &buf, ui8Data );			\
	LL_USART_ClearFlag_RXNE( UART_OF(u) );		\
      }									\
    if ( LL_USART_IsActiveFlag_ORE( UART_OF(u) ) )		\
      {									\
	LL_USART_ClearFlag_ORE( UART_OF(u) );		\
      }									\
  }


#if 0
#if 0
void hw_timer2_init( void )
{
    GPIO_InitTypeDef         GPIO_InitStructure;
    //    TIM_InitTypeDef          TIM_InitStructure;
    TIM_ICInitTypeDef        TIM_ICInitStructure;
    //    TIM_OCInitTypeDef        TIM_OCInitStructure;
    //    DMA_InitTypeDef          DMA_InitStructure;

    // GPIO Periph clock enable
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    // RCC_AHB1PeriphClockCmd(RCC_AHB1_DMA1, ENABLE);

    // Configure in alternate input mode
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // Configure PA2 in alternate output pushpull mode
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // Select TIM2 as alternate source
    GPIO_PinAFConfig( GPIOB, GPIO_PinSource10, GPIO_AF_TIM2 );
    GPIO_PinAFConfig( GPIOB, GPIO_PinSource11, GPIO_AF_TIM2 );

    // Conf

    TIM_ICStructInit( &TIM_ICInitStructure );
    TIM_ICInit( TIM2, &TIM_ICInitStructure );


}
#endif
#endif
