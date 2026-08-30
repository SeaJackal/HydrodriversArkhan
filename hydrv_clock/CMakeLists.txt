cmake_minimum_required(VERSION 3.10)

set(LIBRARY_NAME HydrodriversClock)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(HYDRV_MCU_FAMILY STREQUAL "F1" OR HYDRV_MCU_FAMILY STREQUAL "F4")
    add_library(${LIBRARY_NAME} ${CMAKE_CURRENT_LIST_DIR}/upper_half/src/hydrv_time.cpp)
    target_link_libraries(${LIBRARY_NAME} PUBLIC HydrolibReturnCodes)
    target_link_libraries(${LIBRARY_NAME} PUBLIC CMSIS)
    target_include_directories(${LIBRARY_NAME} PUBLIC upper_half/include)
    target_link_options(${LIBRARY_NAME} PUBLIC "-Wl,--undefined=_gettimeofday")
    
    if(HYDRV_MCU_FAMILY STREQUAL "F1")
        target_include_directories(${LIBRARY_NAME} PUBLIC f1/include)
    elseif(HYDRV_MCU_FAMILY STREQUAL "F4")
        target_include_directories(${LIBRARY_NAME} PUBLIC f4/include)
    endif()
    
    message(STATUS "${LIBRARY_NAME} target added for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
else()
    message(WARNING "${LIBRARY_NAME} target is not supported for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
endif()
