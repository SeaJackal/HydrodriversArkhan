cmake_minimum_required(VERSION 3.10)

if(HYDRV_MCU_FAMILY STREQUAL "F4")
    # add_subdirectory(upper_half)
    add_subdirectory(lower_half)

    if(HYDRV_BUILD_EXAMPLES)
        add_subdirectory(example)
    endif()
    
    message(STATUS "hydrv_tim subdirectories added for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
else()
    message(WARNING "hydrv_tim subdirectories are not supported for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
endif()
