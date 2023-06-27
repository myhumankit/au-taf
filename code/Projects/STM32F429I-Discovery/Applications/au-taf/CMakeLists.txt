CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SUBDIRS( Bsp )
SUBDIRS( Hal )
# SUBDIRS( sw )

SET( PICOVOICE_LANGUAGE "en")
SET( SUB_PROJECT_NAME picovoice )

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLCD_NORTH")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLCD_EAST")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLCD_SOUTH")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLCD_WEST")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFONT_MAG2")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPICOVOICE_LANGUAGE='\"${PICOVOICE_LANGUAGE}\"'")

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${HALDRIVERS_SOURCE_DIR}/Inc
    ${STDPERIPH_SOURCE_DIR}/inc
    ${UTILITIES_DISCO_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${STM32F4XX_INCLUDE_DIR}
    ${CMSIS_INCLUDE_DIR}
    ${BSP_INCLUDE_DIR}
    ${UTILITIES_DIR}/Fonts
   ../../../Config
    ../../Common/Inc
    ../../Common
    ../../../../stm32/Libraries/STM32F4xx_StdPeriph_Driver/inc
    Bsp/STM32F429I-Discovery
    ${STM32F4_SOURCE_DIR}
    ${PICOVOICE_DIR}/sdk/mcu/include/
    ${PICOVOICE_DIR}/resources/porcupine/demo/mcu/stm32f411/stm32f411e-disco/Inc/
    ${PICOVOICE_DIR}/demo/mcu/stm32f411/stm32f411e-disco/Inc/
    ${PICOVOICE_DIR}/resources/porcupine/include/
    ${PICOVOICE_DIR}/resources/porcupine/demo/mcu/stm32f411/stm32f411e-disco/Drivers/BSP/STM32F411E-Discovery
)

SET(PROJECT_SOURCES
  ${STM32F4_BSP_SOURCE_DIR}/../Components/ili9341/ili9341.c
  ${STM32F4_BSP_SOURCE_DIR}/stm32f429i_discovery.c
  ${STM32F4_BSP_SOURCE_DIR}/stm32f429i_discovery_lcd.c
  ${STM32F4_BSP_SOURCE_DIR}/stm32f429i_discovery_sdram.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_ll_dma.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_ll_dma2d.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_hal_dma.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_ll_fmc.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_hal_gpio.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_ll_gpio.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_hal_ltdc.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_hal_sdram.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_ll_spi.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_hal_spi.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_hal_tim.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_ll_usart.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_ll_rcc.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_hal_rcc.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_hal_rcc_ex.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_hal_cortex.c
#  ${STM32F4_HAL_DRIVER_SOURCE_DIR}/stm32f4xx_hal.c
  ${STM32F4_SOURCE_DIR}/Common/Src/debug.c
  ${STM32F4_SOURCE_DIR}/Common/Src/base.c
  Src/user_config.s 
  Src/main.c 
  Src/pico_multi.c 
  Src/pv_audio_rec.c
  Src/pv_st_f429.c
  Src/serial.c
  Inc/main.h
  Inc/pico.h
  ${PICOVOICE_DIR}/resources/porcupine/demo/mcu/stm32f411/stm32f411e-disco/Src/stm32f4xx_hal_msp.c
  Src/syscalls.c
  Src/sysmem.c
  Inc/stm32f4xx_conf.h
  Inc/stm32f4xx_it.h
  Src/stm32f4xx_it.c
  Src/system_stm32f4xx.c
  ${STARTUP_ASM_FILE}
)

ADD_EXECUTABLE(${SUB_PROJECT_NAME} ${PROJECT_SOURCES} )

ADD_LIBRARY( picovoice_lang STATIC IMPORTED )
SET_TARGET_PROPERTIES( picovoice_lang PROPERTIES IMPORTED_LOCATION
  ${STM32F4_SOURCE_DIR}/Modules/picovoice/sdk/mcu/lib/stm32f411/${PICOVOICE_LANGUAGE}/libpicovoice.a )

ADD_LIBRARY( st_pdm STATIC IMPORTED )
SET_TARGET_PROPERTIES( st_pdm PROPERTIES IMPORTED_LOCATION
  ${STM32F4_SOURCE_DIR}/Middlewares/ST/STM32_Audio/Addons/PDM/Lib/libPDMFilter_CM4_GCC_wc32.a )

TARGET_LINK_LIBRARIES( ${SUB_PROJECT_NAME} picovoice_lang picovoice_bsp_board picovoice_hal_driver st_pdm stm32_common)


STM32_ADD_HEX_BIN_TARGETS(${SUB_PROJECT_NAME})
