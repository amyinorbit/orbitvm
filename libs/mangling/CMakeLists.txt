file(GLOB SRC_FILES *.c)
add_library(OrbitMangling ${SRC_FILES})
target_link_libraries(OrbitMangling OrbitAST)
target_link_libraries(OrbitMangling OrbitUtils)

install(TARGETS OrbitMangling DESTINATION lib)
