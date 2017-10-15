file(GLOB SRC_FILES *.c)
add_library(OrbitUtils STATIC ${SRC_FILES})

install(TARGETS OrbitUtils DESTINATION lib)
