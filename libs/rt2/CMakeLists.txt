file(GLOB SRC_FILES *.c)
add_library(OrbitRT2 STATIC chunk.c memory.c debug.c value_array.c vm.c)
target_link_libraries(OrbitRT2 OrbitUtils)

install(TARGETS OrbitRT2 DESTINATION lib)
