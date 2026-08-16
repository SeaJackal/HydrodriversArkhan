function(hydrv_add_renode_tests TESTED_TARGET TEST_SCRIPT)
    if(HYDRV_RUN_TESTS)
        get_property(HYDRV_RENODE_BOARD_FILE GLOBAL PROPERTY HYDRV_RENODE_BOARD_FILE)
        get_property(HYDRV_PYHTONPATH GLOBAL PROPERTY HYDRV_PYHTONPATH)
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        set(TEST_TARGET_NAME ${TESTED_TARGET}Test)
        add_custom_target(${TEST_TARGET_NAME} ALL
            COMMAND ${CMAKE_COMMAND} -E env
            "PYTHONPATH=${HYDRV_PYHTONPATH}:$ENV{PYTHONPATH}"
            ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SCRIPT} 
            ${HYDRV_RENODE_BOARD_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${TESTED_TARGET}.elf
            COMMENT "Running Renode tests"
            VERBATIM
        )
        add_dependencies(${TEST_TARGET_NAME} ${TESTED_TARGET})
    endif()
endfunction()
