/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include "fonts.h"
#include "user_fonts.h"
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
#include "pv_params_rhino.h"
#include "pv_st_f291.h"

#include "base.h"
#include "debug.h"

#include "sw/constant.h"

//#define MEMORY_BUFFER_SIZE (95 * 1024)
#define MEMORY_BUFFER_SIZE (80 * 1024)

static const char* ACCESS_KEY = "MUPWweW2KKH5xLWpGfO4gYIvQKfsIRDz+0y8wf7mwWZUHl1DdQtsrA=="; //AccessKey string obtained from Picovoice Console (https://picovoice.ai/console/)

static int8_t memory_buffer[MEMORY_BUFFER_SIZE] __attribute__((aligned(16)));

static const float PORCUPINE_SENSITIVITY = 0.75f;
static const float RHINO_SENSITIVITY = 0.5f;
static const float RHINO_ENDPOINT_DURATION_SEC = 1.0f;
static const bool RHINO_REQUIRE_ENDPOINT = true;



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


#include "pico.h"


static void wake_word_callback(void) {
    printf("[wake word]\n");
}

static void inference_callback(pv_inference_t *inference) {
    printf("{\n");
    printf("    is_understood : '%s',\n", (inference->is_understood ? "true" : "false"));
    if (inference->is_understood) {
        printf("    intent : '%s',\n", inference->intent);
        if (inference->num_slots > 0) {
            printf("    slots : {\n");
            for (int32_t i = 0; i < inference->num_slots; i++) {
                printf("        '%s' : '%s',\n", inference->slots[i], inference->values[i]);
            }
            printf("    }\n");
        }
    }
    printf("}\n\n");
    pv_inference_delete(inference);
}

pv_status_t pico_init(pico_context_t *pContext)
{
    pv_status_t status;
    
    status = pv_picovoice_init(
              ACCESS_KEY,
              MEMORY_BUFFER_SIZE,
              memory_buffer,
              sizeof(KEYWORD_ARRAY),
              KEYWORD_ARRAY,
              PORCUPINE_SENSITIVITY,
              wake_word_callback,
              sizeof(CONTEXT_ARRAY),
              CONTEXT_ARRAY,
              RHINO_SENSITIVITY,
              RHINO_ENDPOINT_DURATION_SEC,
              RHINO_REQUIRE_ENDPOINT,
              inference_callback,
              &pContext->handle);
    if (status != PV_STATUS_SUCCESS) {
        printf("Picovoice init failed with '%s'", pv_status_to_string(status));
        return status;
    }


    status = pv_picovoice_context_info(pContext->handle, &pContext->rhino_context);
    if (status != PV_STATUS_SUCCESS) {
        printf("retrieving context info failed with '%s'", pv_status_to_string(status));
        return status;
    }

    status = pv_audio_rec_init();
    if (status != PV_STATUS_SUCCESS) {
        printf("Audio init failed with '%s'", pv_status_to_string(status));
        return status;
    }

    status = pv_audio_rec_start();
    if (status != PV_STATUS_SUCCESS) {
        printf("Recording audio failed with '%s'", pv_status_to_string(status));
        return status;
    }
    return status;
}


pv_status_t pico_process(pico_context_t *pContext, const int16_t *pData)
{
  pv_status_t status = pv_picovoice_process(pContext->handle, pData);
  
  return status;
}
