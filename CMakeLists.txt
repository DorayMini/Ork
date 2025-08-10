cmake_minimum_required(VERSION 3.31)

project(graphl)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(lib)

add_executable(graphl main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC graphl_lib)
