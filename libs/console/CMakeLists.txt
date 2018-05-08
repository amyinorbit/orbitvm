file(GLOB SRC_FILES *.c)
add_library(OrbitConsole ${SRC_FILES})
target_link_libraries(OrbitConsole OrbitCSupport)

install(TARGETS OrbitConsole DESTINATION lib)
