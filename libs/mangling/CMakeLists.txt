add_library(OrbitMangling STATIC demangle.c mangle.c)
target_include_directories(OrbitMangling PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(OrbitMangling OrbitAST OrbitUtils OrbitCSupport)

install(TARGETS OrbitMangling ARCHIVE)
