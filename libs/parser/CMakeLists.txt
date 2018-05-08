file(GLOB SRC_FILES *.c)
add_library(OrbitParser ${SRC_FILES})
target_link_libraries(OrbitParser OrbitUtils)
target_link_libraries(OrbitParser OrbitCSupport)
target_link_libraries(OrbitParser OrbitAST)

install(TARGETS OrbitParser DESTINATION lib)
