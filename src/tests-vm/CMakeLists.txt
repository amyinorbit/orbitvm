file(GLOB SRC_FILES *.c)
add_executable(testVM ${SRC_FILES})
target_link_libraries(testVM orbitrt)
add_test(NAME tests_vm COMMAND testVM)