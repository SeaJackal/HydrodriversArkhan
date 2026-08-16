include(FetchContent)

FetchContent_Declare(
    cmsis_device_1_raw
    GIT_REPOSITORY https://github.com/STMicroelectronics/cmsis-device-f1.git
    GIT_TAG v4.3.5
)

FetchContent_Populate(cmsis_device_1_raw)

add_library(CMSIS-device ${cmsis_device_1_raw_SOURCE_DIR}/Source/Templates/system_stm32f1xx.c
    ${cmsis_device_1_raw_SOURCE_DIR}/Source/Templates/gcc/startup_stm32f103xb.s)
target_include_directories(CMSIS-device SYSTEM PUBLIC ${cmsis_device_1_raw_SOURCE_DIR}/Include)
target_link_libraries(CMSIS-device PUBLIC CMSIS_5)
