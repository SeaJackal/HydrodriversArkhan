set(TARGET_NAME HydrvI2CExample)

add_executable(${TARGET_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp)

target_link_libraries(${TARGET_NAME}
    HydrodriversClock HydrodriversI2C HydrodriversUART HydrolibLogger)

get_property(HYDRV_RENODE_TEST_HELPER GLOBAL PROPERTY HYDRV_RENODE_TEST_HELPER)
include(${HYDRV_RENODE_TEST_HELPER})
hydrv_add_renode_tests(${TARGET_NAME} tests/i2c_sim_test.py)
