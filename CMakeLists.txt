cmake_minimum_required(VERSION 3.12)
project(Orbit VERSION 2019.6 LANGUAGES C)

include_directories(PUBLIC include)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall)

# set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
# set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(DEV_TOOLS_DIR "${CMAKE_SOURCE_DIR}/tools")

install(DIRECTORY include/orbit DESTINATION include)

if (${CMAKE_C_COMPILER_ID} STREQUAL Clang OR ${CMAKE_C_COMPILER_ID} STREQUAL AppleClang)
    add_compile_options (-fcolor-diagnostics)
endif()

 add_compile_definitions(__COMPILER_NAME__="${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")

# Add packages/libraries
add_subdirectory(packages/UniC)
add_subdirectory(packages/Unity EXCLUDE_FROM_ALL)

enable_testing()

# Orbit Components
add_subdirectory(libs)
add_subdirectory(bin)
add_subdirectory(tests)

add_custom_target(run
    COMMAND orbit
    DEPENDS orbit
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_target(debug
    COMMAND lldb orbit
    DEPENDS orbit
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
