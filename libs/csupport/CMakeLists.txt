file(GLOB SRC_FILES *.c)
add_library(OrbitCSupport ${SRC_FILES})
target_link_libraries(OrbitCSupport OrbitUtils)

install(TARGETS OrbitCSupport DESTINATION lib)
