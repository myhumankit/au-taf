CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET( SUB_PROJECT_NAME picovoice_bsp_board )

INCLUDE_DIRECTORIES(
  ${STM32F4_BSP_SOURCE_DIR}
  ${PICOVOICE_DIR}/resources/porcupine/demo/mcu/stm32f411/stm32f411e-disco/Drivers/BSP/STM32F411E-Discovery
)

SET(STM32F4_BSP_SOURCE_DIR   STM32F429I-Discovery)

SET(PROJECT_SOURCES
  ${STM32F4_BSP_SOURCE_DIR}/stm32f411e_discovery_audio.c  
  ${STM32F4_BSP_SOURCE_DIR}/stm32f411e_discovery_audio.h  
  ${PICOVOICE_DIR}/resources/porcupine/demo/mcu/stm32f411/stm32f411e-disco/Drivers/BSP/STM32F411E-Discovery/stm32f411e_discovery.c        
  ${PICOVOICE_DIR}/resources/porcupine/demo/mcu/stm32f411/stm32f411e-disco/Drivers/BSP/STM32F411E-Discovery/stm32f411e_discovery.h
)

ADD_LIBRARY( ${SUB_PROJECT_NAME} STATIC ${PROJECT_SOURCES} )
