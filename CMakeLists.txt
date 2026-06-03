cmake_minimum_required(VERSION 3.20)
project(cobblestone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    link_libraries(stdc++ atomic m)
endif()

add_subdirectory(core)
add_subdirectory(infra)
add_subdirectory(app)
add_subdirectory(cli)

enable_testing()
add_subdirectory(tests)
