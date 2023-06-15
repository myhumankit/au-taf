CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET( SUB_PROJECT_NAME picovoice_bsp_board )

INCLUDE_DIRECTORIES(
  ${STM32F4_BSP_SOURCE_DIR}
)

SET(STM32F4_BSP_SOURCE_DIR   STM32F429I-Discovery)

SET(PROJECT_SOURCES
  ${STM32F4_BSP_SOURCE_DIR}/stm32f411e_discovery_audio.c  
#  ${STM32F4_BSP_SOURCE_DIR}/stm32f411e_discovery_gyroscope.c
#  ${STM32F4_BSP_SOURCE_DIR}/stm32f411e_discovery_accelerometer.c  
  ${STM32F4_BSP_SOURCE_DIR}/stm32f411e_discovery_audio.h  
#  ${STM32F4_BSP_SOURCE_DIR}/stm32f411e_discovery_gyroscope.h
#  ${STM32F4_BSP_SOURCE_DIR}/stm32f411e_discovery_accelerometer.h  
  ${STM32F4_BSP_SOURCE_DIR}/stm32f411e_discovery.c        
  ${STM32F4_BSP_SOURCE_DIR}/stm32f411e_discovery.h
)

ADD_LIBRARY( ${SUB_PROJECT_NAME} STATIC ${PROJECT_SOURCES} )
