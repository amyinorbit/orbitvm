add_library(OrbitCSupport STATIC console.c rcarray.c rcmap.c source.c string.c tokens.c)
target_include_directories(OrbitCSupport PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(OrbitCSupport OrbitUtils m TermUtils UniC)
install(TARGETS OrbitCSupport ARCHIVE)
