file(GLOB SRC_FILES *.c)
add_library(OrbitSource ${SRC_FILES})
target_link_libraries(OrbitSource OrbitUtils)

install(TARGETS OrbitSource DESTINATION lib)
