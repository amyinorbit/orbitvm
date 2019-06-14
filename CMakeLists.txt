cmake_minimum_required(VERSION 3.2)
project(Orbit VERSION 0.1 LANGUAGES C)

option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." FALSE)
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       add_compile_options (-fcolor-diagnostics)
    endif ()
endif ()

enable_testing()

include_directories(PUBLIC include)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(FORCE_COLORED_OUTPUT ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")

set(DEV_TOOLS_DIR "${CMAKE_SOURCE_DIR}/tools")

install(DIRECTORY include/orbit DESTINATION include)
add_subdirectory(libs)
add_subdirectory(bin)
add_subdirectory(tests)