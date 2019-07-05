file(GLOB SRC_FILES *.c)
add_executable(orbit ${SRC_FILES})
target_link_libraries(orbit OrbitUtils OrbitRT2 OrbitParser OrbitSema OrbitAST OrbitCodegen)
install(TARGETS orbit DESTINATION bin)