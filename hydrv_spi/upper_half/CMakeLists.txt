set(LIBRARY_NAME HydrodriversSPI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(HYDRV_MCU_FAMILY STREQUAL "F1" OR HYDRV_MCU_FAMILY STREQUAL "F4")
    add_library(${LIBRARY_NAME} INTERFACE)
    
    target_link_libraries(${LIBRARY_NAME} INTERFACE HydrodriversSPILow)
    target_link_libraries(${LIBRARY_NAME} INTERFACE HydrolibConcepts)
    target_link_libraries(${LIBRARY_NAME} INTERFACE HydrodriversGPIO)
    
    target_include_directories(${LIBRARY_NAME} INTERFACE include)

    message(STATUS "${LIBRARY_NAME} target added for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
else()
    message(WARNING "${LIBRARY_NAME} target is not supported for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
endif()
