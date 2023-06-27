#include <string.h>

#include "stm32f4xx.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_ll_usart.h"
//#include "stm32f4xx_conf.h"
//#include "stm32f4xx_usart.h"

#include "buffer.h"

BUFFER_MANAGEMENT_DEF(1,uint8_t,dummy);

void bufferInit(bufferHeader_t *pBuffer, uint16_t nb)
{
    pBuffer->ui16Length = nb;
    pBuffer->ui16WriteIndex = 0;
    pBuffer->ui16ReadIndex = 0;
}

bool bufferIsEmpty( bufferHeader_t *pBuffer )
{
  return( pBuffer->ui16ReadIndex == pBuffer->ui16WriteIndex );
}


void BufferPutHexa2( void *pBuffer, uint8_t ui8Data)
{
  dummy_t *pDummy = (dummy_t *)pBuffer;
  
  static uint8_t aui8Hexa[] = {
    '0','1','2','3','4','5','6','7',
    '8','9','A','B','C','D','E','F'};
  dummyBufferPush(pDummy, aui8Hexa[(ui8Data >> 4) & 0x0F]);
  dummyBufferPush(pDummy,aui8Hexa[ui8Data & 0x0F]);
}


// UART_RX
void Rx_Loop( void *pBuffer, USART_TypeDef *pUart )
{
  dummy_t *pDummy = (dummy_t *)pBuffer;
  uint8_t ui8Data = 0;

  if ( LL_USART_IsActiveFlag_RXNE(pUart ) ) {
    ui8Data = LL_USART_ReceiveData8( pUart );
    dummyBufferPush( pDummy, ui8Data );
  }
}

// UART_TX
void Tx_Loop( void *pBuffer, USART_TypeDef *pUart )
{
  dummy_t *pDummy = (dummy_t *)pBuffer;
  uint8_t ui8Data = 0;

  if ( !bufferIsEmpty( &(pDummy->header) ) &&
       ( LL_USART_IsActiveFlag_TXE(pUart ) ) ) {
    ui8Data = dummyBufferPop( pDummy );
    LL_USART_TransmitData8( pUart, ui8Data );
    //} else if  ( USART_GetFlagStatus( pUart, UART_FLAG_TXE ) ) {
    //    Buffer_PutHexa8( &TxBuffer, *(uint32_t*)0);
    //    USART_SendData( UART_TX, '=' );
  }
}

