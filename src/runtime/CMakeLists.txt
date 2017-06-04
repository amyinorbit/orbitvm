file(GLOB SRC_FILES *.c)
add_executable(orbit ${SRC_FILES})
target_link_libraries(orbit orbitrt)