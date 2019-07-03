add_library(OrbitCodegen STATIC
    codegen.c
)
target_include_directories(OrbitCodegen PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(OrbitCodegen OrbitRT2 UniC OrbitUtils)
install(TARGETS OrbitCodegen DESTINATION lib)
