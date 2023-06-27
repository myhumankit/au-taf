SET(TOOLCHAIN_PREFIX "/usr")
SET(TARGET_TRIPLET "arm-none-eabi")

SET(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin)
SET(TOOLCHAIN_INC_DIR ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/include)
SET(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/lib)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR arm)

if(NOT DEFINED CMAKE_C_COMPILER AND NOT DEFINED CMAKE_CXX_COMPILER)
  SET( CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-gcc )
  SET( CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-g++)
  SET( CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-gcc)
endif()

SET(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-objcopy CACHE INTERNAL "objcopy tool")

# SET (FPU_OPTIONS "-mfpu=fpv4-sp-d16 -mfloat-abi=softfp " )
SET (FPU_OPTIONS "-mfpu=fpv4-sp-d16 -mfloat-abi=hard " )
SET(CMAKE_EXE_LINKER_FLAGS "-g -Wl,--gc-sections -mthumb -mcpu=cortex-m4 ${FPU_OPTIONS} -mabi=aapcs" CACHE INTERNAL "executable linker flags")

SET(CMAKE_MODULE_LINKER_FLAGS "-mthumb -mcpu=cortex-m4 ${FPU_OPTIONS} -mabi=aapcs" CACHE INTERNAL "module linker flags")
SET(CMAKE_SHARED_LINKER_FLAGS "-mthumb -mcpu=cortex-m4 ${FPU_OPTIONS} -mabi=aapcs" CACHE INTERNAL "shared linker flags")

#SET(CMAKE_C_FLAGS "-DUSE_STDPERIPH_DRIVER -DUSE_FULL_LL_DRIVER -DARM_MATH_CM4 -mthumb -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Wall -std=gnu99 -ffunction-sections -fdata-sections -fomit-frame-pointer -g -O0 -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize" CACHE INTERNAL "c compiler flags")
SET(CMAKE_C_FLAGS "-DUSE_FULL_LL_DRIVER -DARM_MATH_CM4 -mthumb -fno-builtin -mcpu=cortex-m4 ${FPU_OPTIONS} -Wall -std=gnu99 -ffunction-sections -fdata-sections -fomit-frame-pointer -g -O0 -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize" CACHE INTERNAL "c compiler flags")

SET(CMAKE_CXX_FLAGS "-mthumb -fno-builtin -mcpu=cortex-m4 ${FPU_OPTIONS} -Wall -std=c++11 -ffunction-sections -fdata-sections -fomit-frame-pointer -mabi=aapcs -fno-unroll-loops -ffast-math -ftree-vectorize" CACHE INTERNAL "cxx compiler flags")
SET(CMAKE_ASM_FLAGS "-mthumb -mcpu=cortex-m4 ${FPU_OPTIONS}  -x assembler-with-cpp" CACHE INTERNAL "asm compiler flags")


FUNCTION(STM32_ADD_HEX_BIN_TARGETS TARGET)
  SET(FILENAME "${TARGET}")
  ADD_CUSTOM_TARGET(${TARGET}.hex DEPENDS ${TARGET} COMMAND ${CMAKE_OBJCOPY} -Oihex ${FILENAME} ${FILENAME}.hex)
  ADD_CUSTOM_TARGET(${TARGET}.bin DEPENDS ${TARGET} COMMAND ${CMAKE_OBJCOPY} -Obinary ${FILENAME} ${FILENAME}.bin)
ENDFUNCTION()

ENABLE_LANGUAGE(ASM)
