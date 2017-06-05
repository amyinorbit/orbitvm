file(GLOB SRC_FILES *.c)
add_library(OrbitStdLib STATIC ${SRC_FILES})
target_link_libraries(OrbitStdLib OrbitUtils)
target_link_libraries(OrbitStdLib OrbitRuntime)