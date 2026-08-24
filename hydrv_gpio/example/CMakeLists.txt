set(TARGET_NAME HydrvGPIOExample)

add_executable(${TARGET_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp)

target_compile_options(${TARGET_NAME} PRIVATE -save-temps=obj)

target_link_libraries(${TARGET_NAME}
    HydrodriversClock HydrodriversGPIO)

get_property(HYDRV_RENODE_TEST_HELPER GLOBAL PROPERTY HYDRV_RENODE_TEST_HELPER)
include(${HYDRV_RENODE_TEST_HELPER})
hydrv_add_renode_tests(${TARGET_NAME} tests/gpio_sim_test.py)
