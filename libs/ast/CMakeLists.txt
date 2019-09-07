add_library(OrbitAST STATIC
    alloc.c
    builders.c
    context.c
    diag_formatter.c
    diag.c printer.c
    traversal.c
    type.c
    scope.c
    json.c
)
target_include_directories(OrbitAST PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(OrbitAST OrbitUtils OrbitCSupport TermUtils)

install(TARGETS OrbitAST ARCHIVE DESTINATION lib)
