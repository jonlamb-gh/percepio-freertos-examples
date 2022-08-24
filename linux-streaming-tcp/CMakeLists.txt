cmake_minimum_required(VERSION 3.5)

project(linux-streaming-file LANGUAGES C)

set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -O0")
#set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -Og")
set(CMAKE_C_FLAGS_RELEASE "-O3")

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 11)
endif()
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

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
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/port.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/portable/MemMang/heap_3.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/queue.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/tasks.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/timers.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/event_groups.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/stream_buffer.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/list.c)

set(
    TRACING_SOURCES
    ${CMAKE_SOURCE_DIR}/trcStreamingPort.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcKernelPort.c
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/trcStreamingRecorder.c)

add_executable(
    ${PROJECT_NAME}
    main.c
    ${KERNEL_SOURCES}
    ${TCPIP_SOURCES}
    ${TRACING_SOURCES})

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    HEAP3)

target_compile_options(
    ${PROJECT_NAME}
    PRIVATE
    -Wall -Wextra -Werror -Wshadow -Wmissing-include-dirs
    -Wno-unused-parameter
    -Wno-pointer-to-int-cast
    -Wno-sign-compare)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/include
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/Posix
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/include
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/Compiler/GCC
    ${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/Include)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    -lrt
    -lpthread)

set(ALL_SOURCES main.c ${KERNEL_SOURCES} ${TCPIP_SOURCES} ${TRACING_SOURCES})
set(ALL_INCLUDES
    -I${CMAKE_SOURCE_DIR}
    -I${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/include
    -I${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/Posix
    -I${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils
    -I${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/include
    -I${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/Compiler/GCC
    -I${CMAKE_SOURCE_DIR}/FreeRTOS/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/Include)
set(EXPANDED_SOURCES)
foreach(source ${ALL_SOURCES})
    get_filename_component(file_c ${source} NAME_WE)
    set(file_c "${file_c}.expanded.c")
    add_custom_command(
            OUTPUT ${file_c}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            #COMMAND ${CMAKE_C_COMPILER} ${ALL_INCLUDES} -E -dD ${source} > ${CMAKE_BINARY_DIR}/${file_c}
            COMMAND ${CMAKE_C_COMPILER} ${ALL_INCLUDES} -E ${source} > ${CMAKE_BINARY_DIR}/${file_c}
            COMMAND clang-format-12 -i ${CMAKE_BINARY_DIR}/${file_c}
            DEPENDS ${source}
            VERBATIM
    )
    list(APPEND EXPANDED_SOURCES ${CMAKE_BINARY_DIR}/${file_c})
endforeach()

add_custom_target(
    expand_macros
    DEPENDS ${EXPANDED_SOURCES})

#add_custom_target(
#    expand_macros ALL
#    SOURCES main.c ${KERNEL_SOURCES} ${TCPIP_SOURCES} ${TRACING_SOURCES}
#    COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -E
#    VERBATIM)
