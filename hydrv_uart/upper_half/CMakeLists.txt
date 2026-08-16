set(LIBRARY_NAME HydrodriversUART)

add_library(${LIBRARY_NAME} INTERFACE)

target_include_directories(${LIBRARY_NAME} INTERFACE include)

target_link_libraries(${LIBRARY_NAME} INTERFACE HydrolibReturnCodes INTERFACE HydrolibConcepts
    INTERFACE HydrolibRingQueue)

if(BUILD_TESTS)
    target_link_libraries(${LIBRARY_NAME} INTERFACE HydrodriversUARTLowMock)
    add_subdirectory(tests)
else()
    target_link_libraries(${LIBRARY_NAME} INTERFACE HydrodriversUARTLow)
endif()
