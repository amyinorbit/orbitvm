file(GLOB SRC_FILES *.c)
add_executable(TestsRT ${SRC_FILES})
target_link_libraries(TestsRT OrbitRT2 OrbitUtils unity)
add_test(NAME tests_runtime COMMAND TestsRT)