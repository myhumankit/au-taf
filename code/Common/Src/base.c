#include <stdint.h>
#include <stdbool.h>

#include "base.h"

int16_t ctoi( uint8_t c )
{
  if (( c >= '0') && ( c <= '9' ))
    return( c - '0' );

  if (( c >= 'A') && ( c <= 'f' ))
    return( c - 'A' + 10 );
  if (( c >= 'a') && ( c <= 'f' ))
    return( c - 'a' + 10 );

  return( -1 );
}

int16_t htoi(char *p)
{
    int16_t value = 0;
    int16_t c;
    
    c = ctoi(p[0]);
    if (c < -1) {
        return -1;
    }
    value = c << 4;

    c = ctoi(p[1]);
    if (c < -1) {
        return -1;
    }    
    return value + c;
}

static char hexa[] = "0123456789ABCDEF";
  
uint32_t ltoi(char *p,int base)
{
    uint32_t value = 0;
    int i;
    bool done;
    
    do {
        done = false;
        for (i = 0;i < base;i++) {
            if (hexa[i] == *p) {
                value = (value * base) + i;
                p++;
                done = true;
                break;
            }
        }
        if (!done && (base > 10)) {
            for (i = 10;i < base;i++) {
                if ((hexa[i] | 0x20) == *p) {
                    value = (value * base) + i;
                    p++;
                    done = true;
                    break;
                }
            }
        }
    } while (done);
    return value;
}

void fmt_hexa8(uint8_t *pData, uint8_t value)
{
  pData[0] = hexa[0x0F & (value >> 4)];
  pData[1] = hexa[0x0F & value];
}

void fmt_hexa16(uint8_t *pData, uint16_t value)
{
  pData[0] = hexa[0x0F & (value >> 12)];
  pData[1] = hexa[0x0F & (value >> 8)];
  pData[2] = hexa[0x0F & (value >> 4)];
  pData[3] = hexa[0x0F & value];
}

void fmt_hexa32(uint8_t *pData, uint32_t value)
{
  int i;
  
  for (i = 0;i < 8;i++) {
    pData[i] = hexa[0x0F & (value >> (28-4*i))];
  }
}



void put_s(char *text)
{
  char c;

  while ((c = *text++) != 0) {
    put_c(c);
  }
}

void put_hexa8(uint32_t v)
{
  put_c(hexa[(v >> 4) & 0x0F]);
  put_c(hexa[v & 0x0F]);
}

void put_hexa32(uint32_t v)
{
  put_hexa8(v >> 24);
  put_hexa8(v >> 16);
  put_hexa8(v >> 8);
  put_hexa8(v);
}
