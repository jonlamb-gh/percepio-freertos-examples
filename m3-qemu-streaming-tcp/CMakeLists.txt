cmake_minimum_required(VERSION 3.5)

project(m3-qemu-streaming-tcp LANGUAGES C)

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
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/portable/GCC/ARM_CM3/port.c
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
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/portable/GCC/ARM_CM3)

set(
    TCPIP_SOURCES
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_DNS.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_DHCP.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_ARP.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_TCP_WIN.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_Stream_Buffer.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_IP.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_TCP_IP.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_UDP_IP.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_Sockets.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/NetworkInterface/MPS2_AN385/NetworkInterface.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/NetworkInterface/MPS2_AN385/ether_lan9118/smsc9220_eth_drv.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/BufferManagement/BufferAllocation_2.c)
set(
    TCPIP_INCLUDES
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/include
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/Compiler/GCC)

set(
    TRACING_SOURCES
    ${CMAKE_SOURCE_DIR}/src/trcStreamingPort.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcKernelPort.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcStreamingRecorder.c)
set(
    TRACING_INCLUDES
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/Include)

set(
    BSP_SOURCES
    ${CMAKE_SOURCE_DIR}/src/startup.c
    ${CMAKE_SOURCE_DIR}/src/syscall.c)
set(
    BSP_INCLUDES
    ${CMAKE_SOURCE_DIR}/vendor
    ${CMAKE_SOURCE_DIR}/vendor/CMSIS)

add_executable(
    ${PROJECT_NAME}
    src/main.c
    src/timer.c
    src/comms.c
    src/ip.c
    src/stats.c
    src/logging.c
    src/sysexit.c
    src/hooks.c
    ${BSP_SOURCES}
    ${TRACING_SOURCES}
    ${KERNEL_SOURCES}
    ${TCPIP_SOURCES})

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${BSP_INCLUDES}
    ${TRACING_INCLUDES}
    ${KERNEL_INCLUDES}
    ${TCPIP_INCLUDES})

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    HEAP3
    QEMU_SOC_MPS2)

add_custom_target(
    simulate
    DEPENDS ${PROJECT_NAME})

add_custom_command(
    TARGET simulate
    POST_BUILD
    MAIN_DEPENDENCY ${PROJECT_NAME}
    COMMAND
    qemu-system-arm
    -machine mps2-an385 -cpu cortex-m3 -m 16M
    -monitor null -semihosting --semihosting-config enable=on,target=native -serial stdio
    -netdev tap,id=net0,ifname=qemu-net,script=no
    -net nic,macaddr=52:54:00:12:34:AD,model=lan9118,netdev=net0
    -nographic
    -kernel ${PROJECT_NAME}
    COMMENT "Starting QEMU")

add_custom_target(
    simulate-debug
    DEPENDS ${PROJECT_NAME})

add_custom_command(
    TARGET simulate-debug
    POST_BUILD
    MAIN_DEPENDENCY ${PROJECT_NAME}
    COMMAND
    qemu-system-arm
    -s -S
    -machine mps2-an385 -cpu cortex-m3 -m 16M
    -monitor null -semihosting --semihosting-config enable=on,target=native -serial stdio
    -netdev tap,id=net0,ifname=qemu-net,script=no
    -net nic,macaddr=52:54:00:12:34:AD,model=lan9118,netdev=net0
    -nographic
    -kernel ${PROJECT_NAME}
    COMMENT "Starting QEMU in debug mode")

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

print_exec_size()
elf_to_bin()
