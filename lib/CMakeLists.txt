project(ork_lib)

file(GLOB_RECURSE SRCS src/*.cpp)
add_library(ork_lib STATIC ${SRCS})
target_include_directories(ork_lib PUBLIC src)

file(GLOB_RECURSE TEST_SRCS test/*.cpp)

add_executable(ork_tests ${TEST_SRCS})
target_link_libraries(ork_lib PUBLIC fmt::fmt)
target_link_libraries(ork_tests gtest gtest_main ork_lib)
