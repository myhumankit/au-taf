SET (PLATFORM_TEMPLATE_DIR ${STM32F4_SOURCE_DIR}/Projects/STM32F429I-Discovery/Templates_LL )

# SET( LD_FILE ${STM32F4_SOURCE_DIR}/Config/stm32_ram.ld )
SET( LD_FILE ${STM32F4_SOURCE_DIR}/Config/stm32f429_flash.ld )
SET( STARTUP_ASM_FILE ${PLATFORM_TEMPLATE_DIR}/SW4STM32/startup_stm32f429xx.s )

# RAM SET(CMAKE_C_FLAGS "-DVECT_TAB_SRAM -DSTM32F429ZI -DSTM32F429_439xx -DUSE_STDPERIPH_DRIVER -D__FPU_PRESENT=1 -DARM_MATH_CM4 -mthumb -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Wall -std=gnu99 -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize" CACHE INTERNAL "c compiler flags")
SET(CMAKE_C_FLAGS "-g -DSTM32F429ZI -DSTM32F429xx -DHSE_VALUE=8000000 ${CMAKE_C_FLAGS} ")

SET(STM32_FMC_SUPPORT TRUE)

SET(CMAKE_EXE_LINKER_FLAGS "-T ${LD_FILE} ${CMAKE_EXE_LINKER_FLAGS}")
