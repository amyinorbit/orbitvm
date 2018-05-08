file(GLOB SRC_FILES *.c)

add_library(OrbitAST ${SRC_FILES})
target_link_libraries(OrbitAST OrbitConsole)
target_link_libraries(OrbitAST OrbitUtils)
target_link_libraries(OrbitAST OrbitCSupport)

install(TARGETS OrbitAST DESTINATION lib)
