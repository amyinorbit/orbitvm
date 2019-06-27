file(GLOB SRC_FILES *.c)
add_library(OrbitRT2 STATIC ${SRC_FILES})
target_link_libraries(OrbitRT2 OrbitUtils)

install(TARGETS OrbitRT2 DESTINATION lib)
