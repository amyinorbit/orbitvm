file(GLOB SRC_FILES *.c)
add_library(OrbitSema ${SRC_FILES})
target_link_libraries(OrbitSema OrbitUtils)
target_link_libraries(OrbitSema OrbitCSupport)
target_link_libraries(OrbitSema OrbitAST)
target_link_libraries(OrbitSema OrbitConsole)
target_link_libraries(OrbitSema OrbitMangling)

install(TARGETS OrbitSema DESTINATION lib)
