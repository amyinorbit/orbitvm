file(GLOB SRC_FILES *.c)
add_executable(${ORBIT_TOOLCHAIN_PREFIX}orbit ${SRC_FILES})
target_link_libraries(${ORBIT_TOOLCHAIN_PREFIX}orbit OrbitRuntime OrbitStdLib)
install(TARGETS ${ORBIT_TOOLCHAIN_PREFIX}orbit DESTINATION bin)