cmake_minimum_required(VERSION 3.10)

project(HydrodriversArkhan VERSION 1.0)

enable_language(C CXX ASM)

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.deps" CACHE PATH "Base directory for CMake FetchContent downloads")
set(HYDRV_RUN_TESTS "OFF" CACHE BOOL "Run tests")

if(NOT DEFINED HYDRV_MCU_FAMILY)
    message(FATAL_ERROR "HYDRV_MCU_FAMILY variable is not defined. Please set it to F1 or F4.")
endif()

if(NOT HYDRV_MCU_FAMILY STREQUAL "F1" AND NOT HYDRV_MCU_FAMILY STREQUAL "F4")
    message(FATAL_ERROR "Invalid HYDRV_MCU_FAMILY value: ${HYDRV_MCU_FAMILY}. Supported values are: F1, F4")
endif()

message(STATUS "Building for MCU family: ${HYDRV_MCU_FAMILY}")

add_subdirectory(renode)
add_subdirectory(third_party)

add_subdirectory(hydrv_clock)
add_subdirectory(hydrv_gpio)
# add_subdirectory(hydrv_tim)
# add_subdirectory(hydrv_spi)
# add_subdirectory(hydrv_i2c)
# add_subdirectory(hydrv_uart)

# add_custom_target(compdb ALL
#     COMMAND /bin/sh -c "compdb -p ${CMAKE_BINARY_DIR} list > compile_commands.json && mv compile_commands.json ${CMAKE_BINARY_DIR}/compile_commands.json"
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#     VERBATIM
# )
