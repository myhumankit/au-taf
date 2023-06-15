/**
  ******************************************************************************
  * @file    Touch_Panel/main.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    11-November-2013
  * @brief   This example describes how to configure and use the touch panel 
  *          mounted on STM32F429I-DISCO boards.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include "fonts.h"
#include "main.h"
//#include "timer.h"
#include "serial.h"

#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_dma2d.h"

#include "buffer.h"

#define __PV_LANGUAGE_FRENCH__

#include "pv_picovoice.h"


#include "pv_audio_rec.h"
#include "pv_st_f429.h"

#include "base.h"
#include "debug.h"
#include "pico.h"

#include "Json/constant.h"


#define FLASH_CONFIG_ADDRESS ((uint8_t *)0x081C0000)

//#define MEMORY_BUFFER_SIZE (95 * 1024)
// #define MEMORY_BUFFER_SIZE (80 * 1024)

// static const char* ACCESS_KEY = "MUPWweW2KKH5xLWpGfO4gYIvQKfsIRDz+0y8wf7mwWZUHl1DdQtsrA=="; //AccessKey string obtained from Picovoice Console (https://picovoice.ai/console/)

// static int8_t memory_buffer[MEMORY_BUFFER_SIZE] __attribute__((aligned(16)));

// static const float PORCUPINE_SENSITIVITY = 0.75f;
// static const float RHINO_SENSITIVITY = 0.5f;
// static const float RHINO_ENDPOINT_DURATION_SEC = 1.0f;
// static const bool RHINO_REQUIRE_ENDPOINT = true;



#define PIXEL_16BITS

#define TRANSPARENT_ENTRY 0x8410
#define BACKGROUND_PLANE  (0xD0000000)
#define FOREGROUND_PLANE  (0xD0028000)

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

BUFFER_MANAGEMENT_DEC(1024,uint8_t,Serial)

extern Serial_t SerialGpsRxBuffer;
extern Serial_t SerialGpsTxBuffer;
extern Serial_t SerialCmdRxBuffer;
extern Serial_t SerialCmdTxBuffer;

/** @addtogroup STM32F429I_DISCOVERY_Examples
  * @{
  */

/** @addtogroup Touch_Panel
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

#define DYNAMIC_ALLOCATION 0
#define BUFFER_SIZE      64
#define U32_BUFFER_SIZE       256

#include "blocks.h"

/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */

void debug_local(void);

void put_c(char c)
{
#if 1
    SerialBufferPush(&SerialCmdTxBuffer, c); 
#endif
}


#define printf myprintf

char *hexa = "0123456789ABCDEF";

void myprintf(char *fmt,...)
{
  va_list ap;
  char c;
  char *(levels[2]);
  int level = 0;
  char buffer[32];
  char *end;
  uint32_t v;
  
  va_start(ap,fmt);
  
  levels[level] = fmt;
  while (((c = *levels[level]++) != 0) || (level != 0)) {
    if (c == 0) {
      level--;
      continue;
    }
    if ((c == '%') && (level == 0)) {
      c = *levels[level]++;
      if (c != '%') {
        int pre = 0;
        int post = 0;
        while ((c >= '0') && (c <= '9')) {
          pre = pre * 10 + c - '0';
          c = *levels[level]++;
        }
        if (c == '.') {
          c = *levels[level]++;
          while ((c >= '0') && (c <= '9')) {
            post = post * 10 + c - '0';
            c = *levels[level]++;
          }
        }
        if (c == 's') {
          level++;
          levels[level] = va_arg(ap,char *);
        } else if (c == 'x') {
          level++;
          end = buffer + sizeof(buffer) - 1;
          *end-- = '\0';
          v = va_arg(ap,uint32_t);
          do {
            *end-- = hexa[v & 0x0F];
            v = v >> 4;
            post--;
          } while ((v != 0) || (post > 0));
          levels[level] = end+1;
        }
        continue;
      }
    }
    put_c(c);
  }
  va_end(ap);
}


void error_handler(void) {
    while(true);
}

#define STM32__DISPLAY__CLEAR 0
#define STM32__DISPLAY__SET_FONT_24 1
#define STM32__DISPLAY__SET_FONT_8 2
#define STM32__DISPLAY__TEXT 3
#define STM32__DISPLAY__SET_COLORS 4

typedef struct {
  int   state;
  int16_t hspace;
  int16_t vspace;
} displayContext_t;

displayContext_t displayContext;


void displayRequestHandler(uint8_t *pData, int16_t length)
{
  int16_t index = 0;
  uint16_t x;
  uint16_t y;
  uint16_t l;
  uint16_t w;
  int16_t  i;

  while (index < length) {
    switch(pData[index]) {
    case STM32__DISPLAY__CLEAR:
      BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      {
        int i;
        uint16_t *p = (uint16_t *)FOREGROUND_PLANE;
        for (i = 0;i < 320*240;i++) {
          *p++ = 0;
        }
      }
      //BSP_LCD_FillRect(0,0,240,320);
      index++;
      break;
    case STM32__DISPLAY__SET_FONT_24:
      BSP_LCD_SetFont(&Font24);
      displayContext.hspace = 16;
      displayContext.vspace = 16;
      index++;
      break;
    case STM32__DISPLAY__SET_FONT_8:
      BSP_LCD_SetFont(&Font8);
      displayContext.hspace = 8;
      displayContext.vspace = 8;
      index++;
      break;
    case STM32__DISPLAY__TEXT:
      y = pData[index+1];
      x = pData[index+2]<<1;
      l = pData[index+3];
      index += 4;
      w = BSP_LCD_GetFont()->Width;
      for (i = 0;i<l;i++) {
	BSP_LCD_DisplayChar(x, y, pData[index++]);
#ifdef FONT_MAG2
	x += 2*w;
#else      
	x += w;
#endif      
#if 0
        BSP_LCD_DisplayCharLandscape(y, x, pData[index++]);
        // BSP_LCD_DisplayChar(y, x, pData[index++]);
        y -= 2 * displayContext.vspace;
        //y -= displayContext.vspace;
#endif	
      }
      break;
    case STM32__DISPLAY__SET_COLORS:
      x = (pData[index+2] << 8) + pData[index+1];
      BSP_LCD_SetTextColor(x);
      x = (pData[index+4] << 8) + pData[index+3];
      BSP_LCD_SetBackColor(x);
      index += 5;
      break;
    default:
      index++;
      break;
    }
  }
}

void displayInit(void)
{
  //displayContext.state = DISPLAY_INITIAL_STATE;
  displayContext.hspace = 16;
}

void displayLoop(void)
{
}

uint8_t l1_frame[256];

uint8_t *get_l2___ptr(uint8_t *l1_ptr)
{
  return l1_ptr;
}
uint8_t *get_l3_ptr(uint8_t *l1_ptr)
{
  return l1_ptr;
}

void displayNLines(
		   char *pStatus,
		   int nLines,
		   char **pLines,
		   int selection)
{
  uint8_t *pData;
  uint16_t count = 0;
  uint8_t l;
  char *s;
  int i;
  int col = 0;
  int previous_col = -1;
  

  pData = get_l3_ptr(l1_frame);

  *pData++ = STM32__DISPLAY__CLEAR;
  count++;

  *pData++ = STM32__DISPLAY__SET_FONT_8;
  count++;

  *pData++ = STM32__DISPLAY__SET_COLORS;
  *pData++ = 0xf8;
  *pData++ = 0x00;
  *pData++ = 0x07;
  *pData++ = 0xff;
  count += 5;
  *pData++ = STM32__DISPLAY__TEXT;
  l = strlen(pStatus);
#if 0  
  *pData++ = 5;
  *pData++ = 155;
#else
  *pData++ = 5;
  *pData++ = 5;
#endif  
  *pData++ = l;
  count += 4;
  memcpy(pData,pStatus,l);
  pData += l;
  count += l;
  
  *pData++ = STM32__DISPLAY__SET_FONT_24;
  count++;


  for (i = 0;i < nLines;i++) {
    s = pLines[i];
    if ( s != NULL) {
      if (selection == i) {
	col = 2;
      } else {
	col = 1;
      }
      if (col != previous_col) {
	*pData++ = STM32__DISPLAY__SET_COLORS;
	if (col == 2) {
	  *pData++ = 0xf8;
	  *pData++ = 0x00;
	} else {
	  *pData++ = 0x07;
	  *pData++ = 0xe0;
	}
	*pData++ = 0x07;
	*pData++ = 0xff;
	count += 5;
	previous_col = col;
      }
      
      *pData++ = STM32__DISPLAY__TEXT;
      l = strlen(s);
#if 0      
      *pData++ = 30+i*40;
      *pData++ = 155;
#else      
      *pData++ = 30+i*40;
      *pData++ = 5;
#endif      
      *pData++ = l;
      count += 4;
      memcpy(pData,s,l);
      pData += l;
      count += l;
    }
  }
  displayRequestHandler(l1_frame,count);
}





#define LEVEL_NB 5
typedef struct {
    uint8_t tokens[LEVEL_NB];
    uint8_t action;
} sequence_t;

typedef struct {
    uint8_t token;
    uint8_t state;
} allowed_t;


typedef struct {
    allowed_t *alloweds;
    uint8_t alloweds_nb;
    uint8_t action;
} state_t;


#define CONTEXT_TOKEN_NB 10

enum {
    STATE_IDLE,
    STATE_COMMAND
};

typedef struct {
  uint32_t update;
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint8_t day;
  uint8_t month;
  uint8_t year;
  uint8_t wday;  
} rtc_context_t;

typedef struct {
  int  serialLine;
  //sem_t sem;
  //l2_context_t L2Context;
  bool power_off_displayed;
  rtc_context_t rtc;
  void *phone_context;
} stm32_context_t;



#define PHONE_N1 5
#define PHONE_N2 5
#define PHONE_N3 5


typedef struct {
  char name[12];
  char number[16];
} directory_leaf_t;

typedef struct {
  char name[12];
} directory_branch_t;

typedef struct {
  directory_branch_t branches_1[PHONE_N1];
  directory_branch_t branches_2[PHONE_N1*PHONE_N2];
  directory_leaf_t leaves[PHONE_N1*PHONE_N2*PHONE_N3];
} directory_t;


typedef struct {
  int level;
  int state;
  uint8_t choices[4];
//  directory_t directory;
//  struct sigaction sig_int_handler;
//  pid_t pid;
  int to_adb[2];
  int from_adb[2];
//  pthread_t adbThreadId;
//  pthread_t waitThreadId;
  char serialNumber[17];
  bool gprs_present;
} phone_context_t;


typedef struct choice {
  uint16_t current_menu_offset;
  uint16_t current_choice;
  uint16_t current_first_choice;
  uint32_t action_id;
  bool     phone;
} choice_t;

typedef struct config {
  uint16_t strings_number;
  uint16_t strings_start_length;
  uint16_t strings_length;
  uint16_t commands_start_number;
  uint16_t commands_start_length;
  uint16_t commands_length;
  uint16_t actions_length;
  uint16_t menus_length;
  uint16_t starting_menu_offset;
  uint16_t word_number;
  uint8_t  lang[2];
  uint16_t access_key_offset;
  uint16_t words_info_offset;
  uint8_t  data[0];
} config_t;

#define CAPTURE_SIZE 1024

typedef struct {
  uint16_t rd_index;
  uint16_t wr_index;
  uint16_t size;
  bool display;
  uint32_t data[CAPTURE_SIZE];
} capture_context_t;

#define COMPARE_SIZE 512

typedef struct {
  uint32_t data[COMPARE_SIZE];
  bool sending;
} compare_context_t;

#define LEVELS_NB 4

typedef struct context {
  uint16_t *pStringsStart;
  char *pStrings;
  uint8_t *pCommandsStart;
  uint8_t *pCommands;
  uint8_t *pActions;
  uint8_t *pMenus;
  config_t *pConfig;
//  pthread_t eventThreadId;
#ifndef MONO  
  int locutor;
#endif  
  bool veille;
  bool phone;
  uint8_t aaa_toggle;
  char status[20];
  uint16_t current_choice;
  uint16_t current_first_choice;
  uint16_t current_choice_level;
  uint16_t current_menu_offset;
  uint32_t action_id;
  uint32_t num_value;
  choice_t choices[LEVELS_NB];
  stm32_context_t stm32_context;
  phone_context_t phone_context;
  capture_context_t capture_context;
  compare_context_t compare_context;
  
} context_t;



void context_init(context_t *pContext)
{
  memset(pContext,0,sizeof(*pContext));
  pContext->veille = true;
  pContext->action_id = 0;
  pContext->phone = false;
  //phoneContextInit(&pContext->phone_context,"df53ee27");
  pContext->stm32_context.phone_context = &pContext->phone_context;
  // 5200b363ec31547e : sam
  // df53ee27 : jj
}

void load_config(context_t *pContext, uint8_t *pConfigRaw)
{
  config_t *pConfig;
  int offset;
  
  pConfig = (config_t *)pConfigRaw;
  
  offset = sizeof(config_t);

  pContext->pStringsStart = (uint16_t *)(offset + (char *)pConfig);
  offset += pConfig->strings_start_length;
  pContext->pStrings = (char *)(offset + (char *)(pConfig));
  offset += pConfig->strings_length;
  pContext->pCommandsStart = (uint8_t *)(offset + (char *)pConfig);
  offset += pConfig->commands_start_length;
  pContext->pCommands = (uint8_t *)(offset + (char *)pConfig);
  offset += pConfig->commands_length;
  pContext->pActions = (uint8_t *)(offset + (char *)pConfig);
  offset += pConfig->actions_length;
  pContext->pMenus = (uint8_t *)(offset + (char *)pConfig);
  offset += pConfig->menus_length;

  pContext->pConfig = pConfig;
  return;
}

int load_picovoice_config(pico_context_t *pContext, uint8_t *pConfigRaw)
{
  config_t *pConfig;
  int offset;
  char *lang = PICOVOICE_LANGUAGE;
  
  pConfig = (config_t *)pConfigRaw;
  
  offset = sizeof(config_t);

  if ((pConfig->lang[0] != lang[0]) || 
      (pConfig->lang[1] != lang[1])) {
      return 1;
  }
  
  pContext->access_key = (int8_t *)(pConfigRaw + pConfig->access_key_offset);
  pContext->num_keywords = pConfig->word_number;
  pContext->sensitivities = (float *)(pConfigRaw + pConfig->words_info_offset);
  pContext->sizes = (int32_t *)(&pContext->sensitivities[pContext->num_keywords]);
  pContext->models = (int32_t *)(&pContext->sizes[pContext->num_keywords]);
  return 0;
}


uint32_t get_uint32(uint8_t *pData)
{
  return(pData[0] +
	 (pData[1] << 8) +
	 (pData[2] << 16) +
	 (pData[3] << 24));
}

uint16_t get_uint16(uint8_t *pData)
{
  return(pData[0] +
	 (pData[1] << 8));
}


void execute(context_t *pContext, uint32_t id)
{
  int n;
  int d;
  int i;
  uint32_t local_id;
  uint16_t start;
  uint8_t c;
  uint8_t byte_1;
  uint8_t buffer[12];
  int counter;

  
  n = pContext->pConfig->commands_start_number;
  d = (n+1)/2;
  i = n / 2;

  counter = 0;
  while (true) {
    local_id = get_uint32(&pContext->pCommandsStart[i*6]);
    if (local_id == id) {
      break;
    }
    
    if (local_id < id) {
      if (d > 2) {
	i += d/2;
      } else {
	i += 1;
      }
      if (i > n) {
	return;
      }
    } else {
      if (d > 2) {
	i -= d/2;
      } else {
	i -= 1;
      }
      if (i < 0) {
	return;
      }
    }
    d = (d+1)/2;
    counter++;
    if (counter > pContext->pConfig->commands_length) {
      fprintf(stderr,"Unable to execute %08X\n",id);
      for (i = 0;i < pContext->pConfig->commands_length;i++) {
	local_id = get_uint32(&pContext->pCommandsStart[i*6]);
	fprintf(stderr,"  %3d %08x\n",i,local_id);
      }
      return;
    }
  }
  
  start = get_uint16(&pContext->pCommandsStart[i*6+4]);
  
  while (pContext->pCommands[start] != COMMAND__END) {
    c = pContext->pCommands[start];
    //printf("C = %02x\n",c);
    switch(c) {
    case COMMAND__BUS:
      buffer[0] = pContext->pCommands[start+2];
      buffer[2] = 0;
      if (pContext->pCommands[start+1] == BUS__BIT_SPECIAL) {
	buffer[1] = pContext->pCommands[start+3];
	start += 4;
      } else {
	buffer[1] = (pContext->pCommands[start+1] << 5) +
	  (pContext->pCommands[start+3] << 3) +
	  pContext->pCommands[start+4];
	start += 5;
      }
      // bt_send_bt(&pContext->stm32_context,buffer);
      //printf(" BUS %02x %02x %02x\n",buffer[0],buffer[1],buffer[2]);
      break;
    case COMMAND__INFRA_ROUGE__PHILIPS:
      if (pContext->pCommands[start+4] == 0) {
	buffer[0] = STM32__INFRARED__PHILIPS;
	buffer[1] = pContext->pCommands[start+3];
	buffer[2] = pContext->pCommands[start+2];
	buffer[3] = pContext->pCommands[start+1];
	//bt_send_ir(&pContext->stm32_context,4,buffer);
      } else if (pContext->pCommands[start+4] == (PHILIPS__n >> 24)) {
	buffer[0] = STM32__INFRARED__PHILIPS;
	buffer[1] = 0x62;
	buffer[2] = 0x00;
	buffer[3] = pContext->num_value;
	//bt_send_ir(&pContext->stm32_context,4,buffer);
      } else if (pContext->pCommands[start+4] == (PHILIPS__nn >> 24)) {
	buffer[0] = STM32__INFRARED__PHILIPS;
	buffer[1] = 0x62;
	buffer[2] = 0x00;
	buffer[3] = pContext->num_value / 10;
	buffer[4] = STM32__INFRARED__PAUSE_MS;
	buffer[5] = 0;
	buffer[6] = 200;
	buffer[7] = STM32__INFRARED__PHILIPS;
	buffer[8] = 0x62;
	buffer[9] = 0x00;
	buffer[10] = pContext->num_value % 10;
	//bt_send_ir(&pContext->stm32_context,11,buffer);
      }	else {
	printf("DD %02X %02X %02X %02X %02X\n",
	       pContext->pCommands[start],
	       pContext->pCommands[start+1],
	       pContext->pCommands[start+2],
	       pContext->pCommands[start+3],
	       pContext->pCommands[start+4]);
      }
      //printf(" IR PHI\n");
      start += 5;
      break;
    case COMMAND__INFRA_ROUGE__AKAI:
      buffer[0] = STM32__INFRARED__AKAI;
      buffer[1] = pContext->pCommands[start+2];
      buffer[2] = pContext->pCommands[start+1];
      //bt_send_ir(&pContext->stm32_context,3,buffer);
      printf(" IR AKAI \n");
      start += 5;
      break;
    case COMMAND__INFRA_ROUGE__RC5:
      buffer[0] = STM32__INFRARED__RC5;
      buffer[1] = pContext->pCommands[start+2];
      buffer[2] = pContext->pCommands[start+1];
      //bt_send_ir(&pContext->stm32_context,3,buffer);
      printf(" IR RC5 \n");
      start += 5;
      break;
    case COMMAND__INFRA_ROUGE__AAA:
      buffer[0] = STM32__INFRARED__AAA;
      buffer[1] = pContext->pCommands[start+2] ^ pContext->aaa_toggle;
      buffer[2] = pContext->pCommands[start+1];
      //bt_send_ir(&pContext->stm32_context,3,buffer);
      pContext->aaa_toggle ^= 0x10;
      printf(" IR AAA \n");
      start += 5;
      break;
    case COMMAND__RF_433__A:
      n = pContext->pCommands[start+1] +
	(pContext->pCommands[start+2] << 8) +
	(pContext->pCommands[start+3] << 16);
      printf("n %08X\n",n);
      pContext->compare_context.sending = true;
      timer_send(n,pContext->compare_context.data,COMPARE_SIZE);
      
      start += 5;
      break;
    case COMMAND__ADB:
      byte_1 = pContext->pCommands[start+1];
      printf(" ADB %02x\n",byte_1);
      switch(byte_1) {
      case 5:
	//phone_hang_up(&pContext->phone_context);
	break;
      case 6:
	//phone_hang_over(&pContext->phone_context);
	break;
      case 7:
	//phone_loudspeaker(&pContext->phone_context);
	break;
      }
      start += 2;
      break;
    default:
      return;
    }
  }
}



void display(context_t *pContext)
{
  int i;
  int j;
  int position = 0;
  uint8_t current_menu_length;
  uint32_t content_offset;
  uint32_t label_id;
  char *label;
  char *lines[5];

  memset(lines,0,sizeof(lines));
  if (pContext->veille) {
    struct timespec current;
    int minutes;
    char current_time[8];

    //get_rtc(&pContext->stm32_context);
    pContext->stm32_context.rtc.hours = 0x12;
    pContext->stm32_context.rtc.minutes = 0x25;

    lines[0] = "VEILLE";
    
    sprintf(current_time," %2x:%02x",
	    pContext->stm32_context.rtc.hours,
	    pContext->stm32_context.rtc.minutes);
    lines[1] = current_time;
    // printf("VEILLE\n");
    displayNLines(
		  "--",
		  5,
		  lines,
		  pContext->current_choice-pContext->current_first_choice);
  } else if (pContext->phone) {
    printf("Display Phone\n");
    //displayPhone(&pContext->phone_context,&pContext->stm32_context);
  } else {
    current_menu_length = pContext->pMenus[pContext->current_menu_offset];
    position = 0;
    for (i = 0;i < pContext->current_choice_level;i++) {
      content_offset = pContext->choices[i].current_menu_offset+1+
	3*pContext->choices[i].current_choice;
      label_id = pContext->pMenus[content_offset];
      for (j = 0;j < 3;j++) {
	pContext->status[position++] = pContext->pStrings[pContext->pStringsStart[label_id]+j];
      }
      pContext->status[position++] = ' ';
    }
    pContext->status[position] = '\0';
    for (i = 0;i < 5;i++) {
      if (pContext->current_first_choice + i < current_menu_length) {
	content_offset = pContext->current_menu_offset+1+3*(i+pContext->current_first_choice);
	label_id = pContext->pMenus[content_offset];
	//printf("Label %d %d\n",label_id,pContext->pStringsStart[label_id]);
	label = &pContext->pStrings[pContext->pStringsStart[label_id]];
	lines[i] = label;
#ifdef DEBUG
	if (pContext->current_first_choice + i == pContext->current_choice) {
	  printf(">%s<\n",label);
	} else {
	  printf("%s\n",label);
	}
#endif	
      } else {
	printf("\n");
      }
    }
    displayNLines(
		  pContext->status,
		  5,
		  lines,
		  pContext->current_choice-pContext->current_first_choice);
  }
}

void back(context_t *pContext)
{
  if (pContext->veille == false) {
    if (pContext->phone) {
    } else if (pContext->current_choice_level > 0) {
      pContext->current_choice_level--;
      
      pContext->current_menu_offset = pContext->choices[pContext->current_choice_level].current_menu_offset;
      pContext->current_choice = pContext->choices[pContext->current_choice_level].current_choice;
      pContext->current_first_choice = pContext->choices[pContext->current_choice_level].current_first_choice;
      pContext->action_id = pContext->choices[pContext->current_choice_level].action_id;
      pContext->phone = pContext->choices[pContext->current_choice_level].phone;
    } else {
      pContext->veille = true;
      pContext->action_id = 0;
    }
  }
}



void process(context_t *pContext, int command)
{
  uint8_t current_menu_length;
  uint16_t offset;
  uint16_t action_offset;
  uint8_t cmd;
  uint16_t menu_id;
  uint8_t choice_id;
  uint32_t action_id;

  // display_context(pContext);
  
  if (pContext->veille) {
    if (command == CMD_COMMAND) {
#ifndef MONO
      pContext->locutor = command;
#endif    
      pContext->current_choice_level = 0;
      pContext->veille = false;
      pContext->current_menu_offset = pContext->pConfig->starting_menu_offset;
      pContext->current_choice = 0;
      pContext->current_first_choice = 0;
    }
  } else if (pContext->phone) {
    int level;
    
    level = pContext->phone_context.level;
    if (command == CMD_NEXT) {
      if (pContext->phone_context.choices[level] >= LEVELS_NB) {
        pContext->phone_context.choices[level] = 0;
      } else {
        pContext->phone_context.choices[level]++;
      }
    } else if (command == CMD_OK) {
      if (level == 2) {
        uint16_t c;
        c = pContext->phone_context.choices[0];
        c = c * PHONE_N1 + pContext->phone_context.choices[1];
        c = c * PHONE_N2 + pContext->phone_context.choices[2];
        // phone_dial(&pContext->phone_context,c);
        /* Get out phone mode */
        pContext->phone = false;
      } else {
        pContext->phone_context.level++;
      }
    } else if (command == CMD_BACK) {
      if (level == 0) {
        pContext->phone = false;
      } else {
        pContext->phone_context.level--;
      }
    }
  } else {
    if (command == CMD_NEXT) {
      current_menu_length = pContext->pMenus[pContext->current_menu_offset];
      //print("%d %d %d" % (current_menu_length,pContext->current_choice,pContext->current_first_choice))
      if (pContext->current_choice + 1 >= current_menu_length) {
        pContext->current_choice = 0;
        pContext->current_first_choice = 0;
      } else {
        pContext->current_choice += 1;
        if (pContext->current_choice >= pContext->current_first_choice + 5) {
          pContext->current_first_choice = pContext->current_choice - 4;
        }
      }
    } else if (command == CMD_OK) {
      offset = pContext->current_menu_offset+1+3*pContext->current_choice;
      action_offset = pContext->pMenus[offset+1] +
        (pContext->pMenus[offset+2] << 8);
      while (pContext->pActions[action_offset] != 0x00) {
        cmd = pContext->pActions[action_offset];
        // printf("cmd %2d\n",cmd);
        switch (cmd) {
        case ACTION__MENU:
          menu_id = pContext->pActions[action_offset+1] +
            (pContext->pActions[action_offset+2] << 8);
          pContext->choices[pContext->current_choice_level].current_menu_offset =
            pContext->current_menu_offset;
          pContext->choices[pContext->current_choice_level].current_choice =
            pContext->current_choice;
          pContext->choices[pContext->current_choice_level].current_first_choice =
            pContext->current_first_choice;
          pContext->choices[pContext->current_choice_level].action_id =
            pContext->action_id;
          pContext->choices[pContext->current_choice_level].phone =
            pContext->phone;
          
          choice_id = pContext->pMenus[offset];
          pContext->action_id = pContext->action_id * (pContext->pConfig->strings_number+1) + choice_id + 1;
          pContext->current_menu_offset = menu_id;
          pContext->current_choice = 0;
          pContext->current_first_choice = 0;
          pContext->current_choice_level += 1;
          action_offset += 3;
          break;
        case ACTION__ACTION:
          choice_id = pContext->pMenus[offset];
          action_offset += 1;
          action_id = pContext->action_id * (pContext->pConfig->strings_number+1) + choice_id + 1;
          //printf("ACTION_ID %08x",action_id);
          execute(pContext,action_id);
          break;
        case ACTION__PHONE:
          //printf("Passage en mode PHONE\n");
          pContext->phone = true;
          pContext->phone_context.level = 0;
          memset(&pContext->phone_context.choices,0,sizeof(pContext->phone_context.choices));
          action_offset += 1;
          break;
        case ACTION__STAY:
          action_offset += 1;
          break;
        case ACTION__SLEEP:
          pContext->veille = true;
          pContext->action_id = 0;
          action_offset += 1;
          break;
        case ACTION__BACK:
          back(pContext);
          action_offset += 1;
          break;
        case ACTION__MAIN:
          pContext->current_choice_level = 0;
          pContext->current_menu_offset = pContext->pConfig->starting_menu_offset;
          pContext->action_id = 0;
          pContext->current_choice = 0;
          pContext->current_first_choice = 0;
          action_offset += 1;
          break;
        case ACTION__ACTION_n:
          pContext->num_value = 0;
          if (pContext->current_choice_level >= 1) {
            pContext->num_value = pContext->current_choice + 1;
          }
          action_offset += 1;
          //printf("Value %d %d\n",pContext->num_value,pContext->action_id);
          action_id = pContext->action_id;
          //printf("ACTION_ID %08x",action_id);
          execute(pContext,action_id);
          break;
        case ACTION__ACTION_nn:
          pContext->num_value = 0;
          if (pContext->current_choice_level >= 1) {
            pContext->num_value = pContext->choices[pContext->current_choice_level-1].current_choice;
            pContext->num_value = (pContext->num_value * 10) +
              pContext->current_choice;
          }
          //printf("Value %d %d\n",pContext->num_value,pContext->action_id);
          action_id = pContext->action_id / (pContext->pConfig->strings_number+1);
          //printf("ACTION_ID %08x",action_id);
          execute(pContext,action_id);
          action_offset += 1;
          break;
        }
      }
    } else if (command == CMD_BACK) {
      back(pContext);
    }
  }
}





context_t myContext;
//param_t myParameters;

//extern uint8_t _binary_config_bin_start[];
//extern uint32_t _binary_config_bin_size;

int main(void)
{
  uint8_t previous_s;
  int32_t  count = 0;
  pv_status_t status;
  pico_context_t pico_context;
  
  status = pv_board_init();
  if (status != PV_STATUS_SUCCESS) {
    error_handler();
  }
  
  /*!< At this stage the microcontroller clock setting is already configured, 
  this is done through SystemInit() function which is called from startup
  file (startup_stm32f429_439xx.s) before to branch to application main.
  To reconfigure the default setting of SystemInit() function, refer to
  system_stm32f4xx.c file
  */

  // ---------- SysTick timer -------- //
  if (SysTick_Config(SystemCoreClock / 1000)) 
    {
      // Capture error
      while (1)
	{
	}
    }

  /* remap vector table */
  if (( 0xFF000000 & (uint32_t)main ) ==
      ( 0xFF000000 & (uint32_t)&count ))
    {
      *(uint32_t *)0x40013800 = 3;
      *(uint32_t *)0xE000ED08 = 0;
    }
    
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  
  //BSP_SDRAM_Init();
  
  BSP_SDRAM_Initialization_sequence(REFRESH_COUNT);
  
  /* LCD initialization */
  BSP_LCD_Init();
  
  /* LCD Layer initialization */
  BSP_LCD_LayerDefaultInit(LCD_BACKGROUND_LAYER,BACKGROUND_PLANE);
  BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER,FOREGROUND_PLANE);

  BSP_LCD_SetColorKeying(LCD_FOREGROUND_LAYER,0x848284);

  {
    int i;
    for (i = 0;i < DISPLAY_WIDTH * DISPLAY_HEIGHT;i++) {
      *(uint16_t *)(FOREGROUND_PLANE + 2 * i) = TRANSPARENT_ENTRY;
    }
  }

  /* Enable the LTDC */
  //LTDC_Cmd(ENABLE);
  //__HAL_LTDC_ENABLE(LTDC1);
  
  
  /* Set LCD foreground layer */
  BSP_LCD_SetLayerVisible(LCD_FOREGROUND_LAYER, ENABLE);
  BSP_LCD_SetLayerVisible(LCD_BACKGROUND_LAYER, ENABLE);  
  
  BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);

  BSP_LCD_SetFont(&Font24);

  BSP_LCD_SetBackColor(TRANSPARENT_ENTRY);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK); 

  serialInit();
  
  
  spiInit();
  
  l3gd20_set_three_state();

  timerInit(&myContext.capture_context.data,CAPTURE_SIZE);
  
  debug_init();
  
  debug_local();
  
  debug_start();

  displayInit();
  
  context_init(&myContext);

  {
    context_t *pContext = &myContext;

    pContext->capture_context.size = CAPTURE_SIZE;
    pContext->capture_context.wr_index = 0;
    pContext->capture_context.rd_index = 0;
    pContext->capture_context.display = false;
    pContext->compare_context.sending = false;
  }
  
  //load_params("params.bin",&myParameters);

  //strncpy(myContext.phone_context.serialNumber,
	//  myParameters.adbId,
	//  sizeof(myContext.phone_context.serialNumber));
  
  load_config(&myContext,FLASH_CONFIG_ADDRESS);


  //load_directory(&myContext.phone_context);
  
  display(&myContext);

  const uint8_t *board_uuid = pv_get_uuid();
  printf("UUID: ");
  for (uint32_t i = 0; i < pv_get_uuid_size(); i++) {
      printf(" %.2x", board_uuid[i]);
  }
  
  printf("\r\n");

  if (load_picovoice_config(&pico_context,FLASH_CONFIG_ADDRESS) != 0) {
    char *lines[] = {"","BAD","LANGUAGE","",""};
    displayNLines(
		  "ERROR",
		  5,
		  lines,
		  0);
  }
  pico_context.handle = NULL;
  pico_context.rhino_context = NULL;

  status = pico_init(&pico_context);
  
  if (status != PV_STATUS_SUCCESS) {
      printf("Recording audio failed with '%s'", pv_status_to_string(status));
      error_handler();
  }
  

  while (1)
  {
    const int16_t *buffer = pv_audio_rec_get_new_buffer();
    if (buffer) {
      status = pico_process(&pico_context,buffer);
      if (status != PV_STATUS_SUCCESS) {
        printf("Picovoice process failed with '%s'", pv_status_to_string(status));
        error_handler();
      }
    }
    
    debug_loop();
    
    displayLoop();

    timerLoop();
 
    if ( USART1->SR & 0x0F) {
        USART1->DR;
    }
    if ( DMA1->HISR & 0x3F00) {
        /* Error on stream 5 */
        DMA1->HIFCR = (DMA1->HISR & 0x3F00);
	LL_DMA_DisableStream( DMA1, LL_DMA_STREAM_5 );
        //DMA_Cmd( DMA1_Stream5, ENABLE );
    }
    if ( DMA2->LISR & 0x3F0000) {
        /* Error on stream 2 */
        DMA2->LIFCR = (DMA2->LISR & 0x3F0000);
	LL_DMA_EnableStream( DMA2, LL_DMA_STREAM_2 );
        //DMA_Cmd( DMA2_Stream2, ENABLE );
    }
    //displayLoop();
    
    SerialCmdRxBuffer.header.ui16WriteIndex = sizeof(SerialCmdRxBuffer.pData);
    SerialCmdRxBuffer.header.ui16WriteIndex -= DMA2_Stream2->NDTR;
    
    if (USART1->SR & USART_FLAG_TXE) {
        int c;
        c = debug_get_next_char();
        if (c != -1) {
            USART1->DR = c;
        } else if (bufferIsEmpty(&SerialCmdTxBuffer.header) == 0) {
          uint8_t c;
          c = SerialBufferPop(&SerialCmdTxBuffer);
          debug_putc(c);
        }
    }
    if (bufferIsEmpty(&SerialCmdRxBuffer.header) == 0) {
      uint8_t c;
      c = SerialBufferPop(&SerialCmdRxBuffer);
      debug_add_char(c);
    }

  }
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif



int debug_clear_handler(debug_context_t *pContext)
{
    uint8_t command[] = { STM32__DISPLAY__CLEAR};

    displayRequestHandler(command,sizeof(command));
    return 0;
}

int debug_test_handler(debug_context_t *pContext)
{
    uint16_t x;
    uint16_t y;
    
    uint8_t command[] = { STM32__DISPLAY__SET_FONT_24,
      STM32__DISPLAY__TEXT,
      0,
      0>>1,
      2,'-','X'};
    if (debug_next_field() == 0) {
        return 0;
    }
    x = ltoi((char *)&pContext->line.data[pContext->field_start],16);
    if (debug_next_field() == 0) {
        return 0;
    }
    y = ltoi((char *)&pContext->line.data[pContext->field_start],16);

    command[2] = x & 0xff;
    command[3] = y >> 1;
    
    displayRequestHandler(command,sizeof(command));
    return 0;
}

int debug_color_handler(debug_context_t *pContext)
{
    uint16_t fore;
    uint16_t back;
    uint8_t command[] = { STM32__DISPLAY__SET_COLORS,
      0,0,0,0
    };
    
    if (debug_next_field() == 0) {
        return 0;
    }
    fore = ltoi((char *)&pContext->line.data[pContext->field_start],16);
    if (debug_next_field() == 0) {
        return 0;
    }
    back = ltoi((char *)&pContext->line.data[pContext->field_start],16);
    
    command[1] = fore & 0xff;
    command[2] = (fore >> 8) & 0xff;
    command[3] = back & 0xff;
    command[4] = (back >> 8) & 0xff;
    
    displayRequestHandler(command,sizeof(command));
    return 0;
}

int debug_lcd_on_handler(debug_context_t *pContext)
{
    BSP_LCD_DisplayOn();
    return 0;
}
int debug_lcd_off_handler(debug_context_t *pContext)
{
    BSP_LCD_DisplayOff();
    return 0;
}

int debug_capture_handler(debug_context_t *pContext)
{
  context_t *pUserContext;
  int enable = 0;
  
  if (debug_next_field() == 0) {
    return 0;
  }
  enable = ltoi((char *)&pContext->line.data[pContext->field_start],16);
  pUserContext = pContext->user_context;

  /* 
   * Initialize pointers to current values 
   */
  pUserContext->capture_context.wr_index = pUserContext->capture_context.size - DMA1_Stream4->NDTR;
  pUserContext->capture_context.rd_index = pUserContext->capture_context.wr_index;
  pUserContext->capture_context.display = enable;
  return 0;
}

int debug_compare_handler(debug_context_t *pContext)
{
  context_t *pUserContext;
  int code = 0;
  
  if (debug_next_field() == 0) {
    return 0;
  }
  code = ltoi((char *)&pContext->line.data[pContext->field_start],16);
  pUserContext = pContext->user_context;

  pUserContext->compare_context.sending = true;
  timer_send(code,pUserContext->compare_context.data,COMPARE_SIZE);
  return 0;
}

int debug_date_handler(debug_context_t *pContext)
{
  context_t *pUserContext;
  int code = 0;
  
  if (debug_next_field() == 0) {
    return 0;
  }
  code = ltoi((char *)&pContext->line.data[pContext->field_start],16);
  pUserContext = pContext->user_context;

  pUserContext->compare_context.sending = true;
  timer_send(code,pUserContext->compare_context.data,COMPARE_SIZE);
  return 0;
}

int debug_time_handler(debug_context_t *pContext)
{
  context_t *pUserContext;
  int code = 0;
  
  if (debug_next_field() == 0) {
    return 0;
  }
  code = ltoi((char *)&pContext->line.data[pContext->field_start],16);
  pUserContext = pContext->user_context;

  pUserContext->compare_context.sending = true;
  timer_send(code,pUserContext->compare_context.data,COMPARE_SIZE);
  return 0;
}

int debug_spi_handler(debug_context_t *pContext)
{
  context_t *pUserContext;
  int code = 0;
  int i;
  uint8_t data[5];

  for (i = 0;i < 5;i++) {
    if (debug_next_field() == 0) {
      break;
    }
    data[i] = ltoi((char *)&pContext->line.data[pContext->field_start],16);
  }
  spi_send(data,i);
  return 0;
}


int hotword_handler(int hot)
{
    process(&myContext,hot);
    display(&myContext);
    return 0;
}

int debug_hotword_handler(debug_context_t *pContext)
{
    uint16_t hot;
    
    if (debug_next_field() == 0) {
        return 0;
    }
    hot = ltoi((char *)&pContext->line.data[pContext->field_start],16);
    
    process(&myContext,hot);
    display(&myContext);
    return 0;
}
  
static debug_command_t locals[] = {
    { .name = "color",
      .help = "Defines fore and back ground colors",
      .handler = debug_color_handler,
      .next = NULL},
    { .name = "clear",
      .help = "Clears the display",
      .handler = debug_clear_handler,
      .next = NULL},
    { .name = "capture",
      .help = "0/1 Disable/Enable the display of captured data",
      .handler = debug_capture_handler,
      .next = NULL},
    { .name = "compare",
      .help = "code Send code on RF433",
      .handler = debug_compare_handler,
      .next = NULL},
    { .name = "hotword",
      .help = "n : Acts as hotword n is detected",
      .handler = debug_hotword_handler,
      .next = NULL},
    { .name = "text",
      .help = "Tests the display",
      .handler = debug_test_handler,
      .next = NULL},
    { .name = "date",
      .help = "Set RTC date YYYY/MM/DD",
      .handler = debug_date_handler,
      .next = NULL},
    { .name = "spi",
      .help = "Send up to 5 bytes on SPI",
      .handler = debug_spi_handler,
      .next = NULL},
    { .name = "time",
      .help = "Set RTC time hh:mm",
      .handler = debug_time_handler,
      .next = NULL},
    { .name = "lcd_on",
      .help = "enables lcd display",
      .handler = debug_lcd_on_handler,
      .next = NULL},
    { .name = "lcd_off",
      .help = "disables lcd display",
      .handler = debug_lcd_off_handler,
      .next = NULL}
};

void debug_local(void)
{
    //memset(&local_command_context,0,sizeof(local_command_context));
    debug_add_commands(locals,SIZEOF(locals));
    debug_set_user_context(&myContext);
}
void timerLoop()
{
  context_t *pContext = &myContext;

  if (pContext->capture_context.display) {
    pContext->capture_context.wr_index = pContext->capture_context.size - DMA1_Stream4->NDTR;
    if ((pContext->capture_context.wr_index != pContext->capture_context.rd_index) &&
	(debug_tx_empty())) {
      uint32_t value;
      uint8_t text[16];

      value = pContext->capture_context.data[pContext->capture_context.rd_index];
      pContext->capture_context.rd_index = (pContext->capture_context.rd_index + 1) & (pContext->capture_context.size - 1);
      fmt_hexa32(text,value);
      text[8] = '\n';
      text[9] = '\0';
      debug_puts(text);
    }
  }
  if (pContext->compare_context.sending) {
    if ((DMA1->LISR & 0x2C) &&
	((int32_t)(TIM5->CNT-TIM5->CCR3) > 100)) {
	LL_DMA_DisableStream( DMA1, LL_DMA_STREAM_0 );
        /* Error or end on stream 0 */
        DMA1->LIFCR = (DMA1->LISR & 0x3F);
	LL_TIM_CC_DisableChannel(TIM5,LL_TIM_CHANNEL_CH3);
	{
	  LL_GPIO_InitTypeDef   GPIO_InitStructure;
	  
	    GPIO_InitStructure.Pin = LL_GPIO_PIN_2;
	    GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
	    GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	    GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	    GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
	    GPIO_InitStructure.Alternate = LL_GPIO_AF_2;
	    LL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	}
	
        //DMA_Cmd( DMA2_Stream2, ENABLE );
	pContext->compare_context.sending = false;
    }
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/*
 * Info en flash :
 * - palette 1 Ko
 * - cartes 64 Ko par carte (nb cartes ~300 niv 18 )
 * - table par niveau 
 */

