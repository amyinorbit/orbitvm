add_library(OrbitCodegen STATIC builder.c codegen.c)
target_include_directories(OrbitCodegen PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(OrbitCodegen OrbitAST OrbitRT2 OrbitUtils)
install(TARGETS OrbitCodegen ARCHIVE DESTINATION lib)
