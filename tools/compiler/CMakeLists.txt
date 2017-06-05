file(GLOB SRC_FILES *.c)
add_executable(orbitc ${SRC_FILES})
target_link_libraries(orbitc OrbitParser)
install(TARGETS orbitc DESTINATION bin)