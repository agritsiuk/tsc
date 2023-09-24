enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(TscTest TscTest.cpp)
target_link_libraries(TscTest PRIVATE tsc gtest gtest_main)
target_compile_options(TscTest PRIVATE -Wall -Werror -Wextra -Wpedantic)
add_test(NAME TscTest COMMAND TscTest)
