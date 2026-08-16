include(FetchContent)

FetchContent_Declare(
    cmsis_5_raw
    GIT_REPOSITORY https://github.com/ARM-software/CMSIS_5.git
    GIT_TAG 5.9.0
)

FetchContent_Populate(cmsis_5_raw)

add_library(CMSIS_5 INTERFACE)

target_include_directories(CMSIS_5 SYSTEM INTERFACE ${cmsis_5_raw_SOURCE_DIR}/CMSIS/Core/Include)
