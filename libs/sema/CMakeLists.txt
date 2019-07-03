add_library(OrbitSema helpers.c typecheck.c errors.c expr_resolver.c)
target_include_directories(OrbitRT2 PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(OrbitSema OrbitUtils OrbitCSupport OrbitAST OrbitMangling)
install(TARGETS OrbitSema DESTINATION lib)
