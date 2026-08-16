include(FetchContent)

FetchContent_Declare(
    cmsis_device_4_raw
    GIT_REPOSITORY https://github.com/STMicroelectronics/cmsis-device-f4.git
    GIT_TAG v2.6.11
)

FetchContent_Populate(cmsis_device_4_raw)

add_library(CMSIS-device ${cmsis_device_4_raw_SOURCE_DIR}/Source/Templates/system_stm32f4xx.c
    ${cmsis_device_4_raw_SOURCE_DIR}/Source/Templates/gcc/startup_stm32f407xx.s)
target_include_directories(CMSIS-device SYSTEM PUBLIC ${cmsis_device_4_raw_SOURCE_DIR}/Include)
target_link_libraries(CMSIS-device PUBLIC CMSIS_5)
