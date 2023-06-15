/*
    Copyright 2020-2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "stm32f429i_discovery.h"

//#define __PV_LANGUAGE_FRENCH__
#define __PV_LANGUAGE_ENGLISH__

#include "pv_porcupine_mcu.h"

#include "pv_audio_rec.h"
#include "pv_params_multi.h"
#include "pv_st_f429.h"

#include "pico.h"

#define MEMORY_BUFFER_SIZE (100 * 1024)

//static const char* ACCESS_KEY = "MUPWweW2KKH5xLWpGfO4gYIvQKfsIRDz+0y8wf7mwWZUHl1DdQtsrA=="; //AccessKey string obtained from Picovoice Console (https://picovoice.ai/console/)

static int8_t memory_buffer[MEMORY_BUFFER_SIZE] __attribute__((aligned(16)));

#if 0
static const int32_t NUM_KEYWORDS = 4;
static const int32_t KEYWORD_MODEL_SIZES[] = {
        sizeof(DEFAULT_KEYWORD_ARRAY),
        sizeof(PICOVOICE_KEYWORD_ARRAY),
        sizeof(BUMBLEBEE_KEYWORD_ARRAY),
        sizeof(ALEXA_KEYWORD_ARRAY),
};
static const void *KEYWORD_MODELS[] = {
        DEFAULT_KEYWORD_ARRAY,
        PICOVOICE_KEYWORD_ARRAY,
        BUMBLEBEE_KEYWORD_ARRAY,
        ALEXA_KEYWORD_ARRAY,
};
static const float SENSITIVITIES[] = {
        0.75f,
        0.75f,
        0.75f,
        0.75f,
};
#endif

static const char *KEYWORDS_NAME[] = {
        "Commande",
        "Suivant",
        "D'accord",
        "Retour",
};

static const int cmds[] =
  {
   CMD_COMMAND,
   CMD_NEXT,
   CMD_OK,
   CMD_BACK
  };
			   
static void wake_word_callback(int32_t keyword_index) {
    printf("[wake word] %s\n", KEYWORDS_NAME[keyword_index]);
    hotword_handler(cmds[keyword_index]);
}

pv_status_t pico_init(pico_context_t *pContext)
{
    pv_status_t status;
  
    status = pv_audio_rec_init();
    if (status != PV_STATUS_SUCCESS) {
        printf("Audio init failed with '%s'", pv_status_to_string(status));
        error_handler();
    }

    status = pv_audio_rec_start();
    if (status != PV_STATUS_SUCCESS) {
        printf("Recording audio failed with '%s'", pv_status_to_string(status));
        error_handler();
    }

    status = pv_porcupine_init(
                pContext->access_key,
                MEMORY_BUFFER_SIZE,
                memory_buffer,
                pContext->num_keywords,
                pContext->sizes,
                pContext->models,
                pContext->sensitivities,
                &pContext->handle);
    return status;
}


pv_status_t pico_process(pico_context_t *pContext, const int16_t *pData)
{
    int32_t keyword_index;
    pv_status_t status = pv_porcupine_process(pContext->handle, pData, &keyword_index);
    if (status != PV_STATUS_SUCCESS) {
        printf("Porcupine process failed with '%s'", pv_status_to_string(status));
        return status;
    }
    if (keyword_index != -1) {
        wake_word_callback(keyword_index);
    }
    return status;
}

