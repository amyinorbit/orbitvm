add_library(OrbitUtils STATIC hashing.c memory.c pack.c utf8.c wcwidth.c)
target_include_directories(OrbitUtils PUBLIC ${PROJECT_SOURCE_DIR}/include)
install(TARGETS OrbitUtils DESTINATION lib)
