file(GLOB SRC_FILES *.c)
add_executable(${ORBIT_TOOLCHAIN_PREFIX}orbitc ${SRC_FILES})
target_link_libraries(${ORBIT_TOOLCHAIN_PREFIX}orbitc OrbitParser)
install(TARGETS ${ORBIT_TOOLCHAIN_PREFIX}orbitc DESTINATION bin)