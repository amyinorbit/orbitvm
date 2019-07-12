add_library(OrbitRT2 STATIC
    memory.c
    map.c
    debug.c
    value.c
    value_object.c
    value_string.c
    vm.c
    gc.c
    invocation.c
    buffer.c
)
target_include_directories(OrbitRT2 PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(OrbitRT2 UniC OrbitUtils)
install(TARGETS OrbitRT2 ARCHIVE)
