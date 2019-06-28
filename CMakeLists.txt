cmake_minimum_required(VERSION 3.12)
project(Orbit VERSION 2019.6 LANGUAGES C)

enable_testing()

include_directories(PUBLIC include)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(DEV_TOOLS_DIR "${CMAKE_SOURCE_DIR}/tools")

install(DIRECTORY include/orbit DESTINATION include)

if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND CMAKE_GENERATOR STREQUAL "Ninja")
    set(CMAKE_C_FLAGS "-fcolor-diagnostics ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "-fcolor-diagnostics ${CMAKE_CXX_FLAGS}")
endif()

add_subdirectory(libs)
add_subdirectory(bin)
add_subdirectory(tests)

add_custom_target(run
    COMMAND orbit2
    DEPENDS orbit2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
