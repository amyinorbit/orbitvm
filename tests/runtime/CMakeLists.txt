file(GLOB SRC_FILES *.c)
add_executable(TestsRT ${SRC_FILES})
target_link_libraries(TestsRT OrbitRuntime OrbitUtils)
add_test(NAME tests_runtime COMMAND TestsRT)