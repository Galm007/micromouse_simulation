cmake_minimum_required(VERSION 3.14)
project(micromouse_simulation)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(raylib QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git)
    FetchContent_MakeAvailable(raylib)
endif()

find_package(raylib_cpp QUIET)
if (NOT raylib_cpp_FOUND)
    include(FetchContent)
    FetchContent_Declare(raylib_cpp GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git)
    FetchContent_MakeAvailable(raylib_cpp)
endif()

file(GLOB SRCS src/*.cpp)
add_executable(${PROJECT_NAME} ${SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE include lib)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib raylib_cpp)
