if(HYDRV_MCU_FAMILY STREQUAL "F4")
    add_subdirectory(upper_half)
    add_subdirectory(lower_half)

    if(HYDRV_BUILD_EXAMPLES)
        add_subdirectory(example)
    endif()

    message(STATUS "HydrodriversI2C target added for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
else()
    message(WARNING "HydrodriversI2C target is not supported for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
endif()
