project(graphl_lib)

file(GLOB_RECURSE SRCS src/*.cpp)
add_library(graphl_lib STATIC ${SRCS}
        src/tokenizer.cpp
        src/tokenizer.h)
target_include_directories(graphl_lib PUBLIC src)

file(GLOB_RECURSE TEST_SRCS test/*.cpp)

add_executable(graphl_tests ${TEST_SRCS})
target_link_libraries(graphl_lib PUBLIC fmt::fmt)
target_link_libraries(graphl_tests gtest gtest_main graphl_lib)
