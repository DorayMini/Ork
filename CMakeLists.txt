cmake_minimum_required(VERSION 3.31)
set(CMAKE_CXX_STANDARD 20)

project(ork)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG e69e5f977d458f2650bb346dadf2ad30c5320281
)
FetchContent_MakeAvailable(fmt)

add_subdirectory(lib)

add_executable(ork main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ork_lib)
