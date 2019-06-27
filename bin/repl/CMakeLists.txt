file(GLOB SRC_FILES *.c)
add_executable(orbit2 ${SRC_FILES})
target_link_libraries(orbit2 OrbitUtils OrbitRT2 OrbitParser OrbitSema OrbitAST)
install(TARGETS orbit2 DESTINATION bin)