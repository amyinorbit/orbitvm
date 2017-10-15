file(GLOB SRC_FILES *.c)
add_library(OrbitRuntime STATIC ${SRC_FILES})
target_link_libraries(OrbitRuntime OrbitUtils)

install(TARGETS OrbitRuntime DESTINATION lib)
