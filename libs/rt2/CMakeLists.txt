add_library(OrbitRT2 STATIC
    chunk.c
    memory.c
    debug.c
    value.c
    value_array.c
    value_object.c
    value_string.c
    vm.c
    gc.c
    invocation.c
)
target_include_directories(OrbitRT2 PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(OrbitRT2 UniC OrbitUtils)
install(TARGETS OrbitRT2 DESTINATION lib)
