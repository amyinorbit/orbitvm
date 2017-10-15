file(GLOB SRC_FILES *.c)
add_library(OrbitConsole ${SRC_FILES})
install(TARGETS OrbitConsole DESTINATION lib)
