add_library(OrbitParser STATIC parser_lexing.c parser_private.c parser.c recursive_descent.c)
target_include_directories(OrbitParser PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(OrbitParser OrbitUtils OrbitCSupport OrbitAST)

install(TARGETS OrbitParser ARCHIVE DESTINATION lib)
