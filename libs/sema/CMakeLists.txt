add_library(OrbitSema helpers.c typecheck.c)
target_link_libraries(OrbitSema OrbitUtils)
target_link_libraries(OrbitSema OrbitCSupport)
target_link_libraries(OrbitSema OrbitAST)
target_link_libraries(OrbitSema OrbitMangling)

install(TARGETS OrbitSema DESTINATION lib)
