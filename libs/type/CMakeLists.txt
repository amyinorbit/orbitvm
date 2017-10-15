file(GLOB SRC_FILES *.c)
add_library(OrbitType ${SRC_FILES})
target_link_libraries(OrbitType OrbitConsole)
target_link_libraries(OrbitType OrbitUtils)

install(TARGETS OrbitType DESTINATION lib)
