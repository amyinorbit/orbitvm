add_test(NAME tests_fixed COMMAND ${DEV_TOOLS_DIR}/testing/test-valid.sh ${CMAKE_CURRENT_LIST_DIR} $<TARGET_FILE:orbitc>)
set_tests_properties(tests_fixed PROPERTIES DEPENDS orbitc)