/*
    Copyright 2020-2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <stdbool.h>
#include <string.h>

#include "stm32f429i_discovery.h"

#include "pv_st_f429.h"

#define UUID_ADDRESS (0x1FFF7A10)
#define UUID_SIZE (12)

static uint8_t uuid[UUID_SIZE];

const uint8_t *pv_get_uuid(void) {
    return (const uint8_t *) uuid;
}

const uint32_t pv_get_uuid_size(void) {
    return UUID_SIZE;
}

pv_status_t pv_board_init() {
    if (HAL_Init() != HAL_OK) {
        return PV_STATUS_INVALID_STATE;
    }

    memcpy(uuid, (uint8_t *) UUID_ADDRESS, UUID_SIZE);
    return PV_STATUS_SUCCESS;
}

void pv_board_deinit() {
}

void pv_error_handler(void) {
    __disable_irq();
    while(true);
}

void assert_failed(uint8_t* file, uint32_t line)
{
    (void) file;
    (void) line;
    pv_error_handler();
}

int __io_putchar(uint8_t ch) {
    ITM_SendChar(ch);
    return ch;
}
