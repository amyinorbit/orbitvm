add_test(NAME tests_fixed COMMAND ${DEV_TOOLS_DIR}/test_valid.sh ${CMAKE_CURRENT_LIST_DIR} $<TARGET_FILE:orbitc>)
set_tests_properties(tests_fixed PROPERTIES DEPENDS orbitc)