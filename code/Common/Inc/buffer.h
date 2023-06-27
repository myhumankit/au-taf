#ifndef BUFFER_H_
#define BUFFER_H_

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint16_t    ui16Length;
  uint16_t    ui16WriteIndex;
  uint16_t    ui16ReadIndex;
} bufferHeader_t;

void bufferInit(bufferHeader_t *pBuffer, uint16_t nb);
bool bufferIsEmpty( bufferHeader_t *pBuffer );

void bufferPutHexa2( void *pBuffer, uint8_t ui8Data);


#define BUFFER_INC_INDEX(pHeader,index)				\
  (pHeader)->index++;						\
  if ( (pHeader)->index >= (pHeader)->ui16Length ) {	        \
    (pHeader)->index = 0;					\
  }                                                             \
  

#define BUFFER_MANAGEMENT_DEC(nb,t,name)                        \
typedef struct {                                                \
  bufferHeader_t header;					\
  t              pData[nb];					\
} name##_t;                                                     \
void name##BufferInit( name##_t *pBuffer);                      \
void name##BufferPush( name##_t *pBuffer, t data );             \
t name##BufferPop( name##_t *pBuffer);

#define BUFFER_MANAGEMENT_DEF(nb,t,name)                        \
BUFFER_MANAGEMENT_DEC(nb,t,name)                                \
 								\
 void name##BufferInit( name##_t *pBuffer )			\
{								\
  bufferInit( &(pBuffer->header), nb);				\
}								\
								\
void name##BufferPush( name##_t *pBuffer, t data )              \
{                                                               \
  if ( NULL == pBuffer) {					\
    return;                                                     \
  }                                                             \
                                                                \
  pBuffer->pData[pBuffer->header.ui16WriteIndex] = data;	\
  BUFFER_INC_INDEX(&pBuffer->header,ui16WriteIndex);            \
}                                                               \
								\
t name##BufferPop( name##_t *pBuffer)                           \
{                                                               \
  t data;                                                       \
                                                                \
  if ( NULL == pBuffer) {					\
    memset(&data,0,sizeof(data));				\
    return data;						\
  }                                                             \
  data = pBuffer->pData[pBuffer->header.ui16ReadIndex];		\
  BUFFER_INC_INDEX(&pBuffer->header,ui16ReadIndex);             \
  return( data );                                               \
}
#endif
