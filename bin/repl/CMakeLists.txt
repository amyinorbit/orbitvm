file(GLOB SRC_FILES *.c)
add_executable(orbit ${SRC_FILES})
target_link_libraries(orbit PRIVATE
    OrbitUtils OrbitRT2 OrbitParser OrbitSema OrbitAST OrbitCodegen TermUtils)
install(TARGETS orbit DESTINATION bin)
