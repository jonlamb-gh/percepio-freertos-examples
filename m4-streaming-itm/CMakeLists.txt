cmake_minimum_required(VERSION 3.5)

project(m4-streaming-itm LANGUAGES C)
enable_language(ASM)

set(
    FREERTOS_TAG
    "202107.00"
    CACHE
    STRING
    "Sets the FreeRTOS git tag to use")

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/FreeRTOS")
    message(STATUS "Cloning FreeRTOS ${FREERTOS_TAG}")
    execute_process(
        COMMAND git clone --depth 1 --recurse-submodules -j 4 --branch "${FREERTOS_TAG}" https://github.com/FreeRTOS/FreeRTOS.git "${CMAKE_SOURCE_DIR}/FreeRTOS"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE GIT_CLONE_RESULT
        OUTPUT_VARIABLE GIT_CLONE_ERROR
        ERROR_VARIABLE GIT_CLONE_ERROR)
    if(GIT_CLONE_RESULT AND NOT GIT_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR "Cloning FreeRTOS ${FREERTOS_TAG} failed:\n${GIT_CLONE_ERROR}")
    endif()
endif()

set(
    KERNEL_SOURCES
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/portable/MemMang/heap_3.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/queue.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/tasks.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/timers.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/event_groups.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/stream_buffer.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/list.c)
set(
    KERNEL_INCLUDES
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/include
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/portable/GCC/ARM_CM4F)

set(
    TRACING_SOURCES
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/streamports/ARM_ITM/trcStreamingPort.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcKernelPort.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcStreamingRecorder.c)
set(
    TRACING_INCLUDES
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/Include
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/streamports/ARM_ITM/include)

set(
    BSP_SOURCES
    ${CMAKE_SOURCE_DIR}/vendor/stm32f4xx/Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c
    ${CMAKE_SOURCE_DIR}/vendor/stm32f4xx/Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f40_41xxx.s
    ${CMAKE_SOURCE_DIR}/vendor/stm32f4xx/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c
    ${CMAKE_SOURCE_DIR}/vendor/stm32f4xx/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c
    ${CMAKE_SOURCE_DIR}/vendor/stm32f4xx/Libraries/STM32F4xx_StdPeriph_Driver/src/misc.c)
set(
    BSP_INCLUDES
    ${CMAKE_SOURCE_DIR}/vendor/stm32f4xx/Project/STM32F4xx_StdPeriph_Templates
    ${CMAKE_SOURCE_DIR}/vendor/stm32f4xx/Libraries/STM32F4xx_StdPeriph_Driver/inc
    ${CMAKE_SOURCE_DIR}/vendor/stm32f4xx/Libraries/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/vendor/stm32f4xx/Libraries/CMSIS/Device/ST/STM32F4xx/Include)

add_executable(
    ${PROJECT_NAME}
    main.c
    ${BSP_SOURCES}
    ${TRACING_SOURCES}
    ${KERNEL_SOURCES})

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${BSP_INCLUDES}
    ${TRACING_INCLUDES}
    ${KERNEL_INCLUDES})

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    HEAP3
    STM32F407xx
    STM32F40_41xxx
    STM32F4XX
    HSE_VALUE=12000000
    USE_STDPERIPH_DRIVER
    __FPU_PRESENT=1
    __FPU_USED=1
    ARM_MATH_CM4)

add_custom_target(
    debug
    DEPENDS
    ${PROJECT_NAME})

add_custom_command(
    TARGET debug
    POST_BUILD
    MAIN_DEPENDENCY ${PROJECT_NAME}
    COMMAND
    gdb-multiarch build/${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Starting gdb session")

add_custom_target(
    flash
    DEPENDS
    ${PROJECT_NAME})

add_custom_command(
    TARGET flash
    POST_BUILD
    MAIN_DEPENDENCY ${PROJECT_NAME}
    COMMAND
    /opt/JLink_Linux_V754b_x86_64/JFlashExe -openprjm4.jflash -openbuild/${PROJECT_NAME}.bin,0x8000000 -auto -startapp -exit
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Flashing target")

print_exec_size()
elf_to_bin()
