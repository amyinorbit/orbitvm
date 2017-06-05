cmake_minimum_required(VERSION 3.2)
project(Orbit VERSION 0.1 LANGUAGES C)

enable_testing()

include_directories(PUBLIC include)

#set(CMAKE_BINARY_DIR "./products")
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")

#add_subdirectory(src)
add_subdirectory(libs)
add_subdirectory(tools)
add_subdirectory(tests)