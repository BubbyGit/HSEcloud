cmake_minimum_required(VERSION 3.10)
project(Tests)

set(CMAKE_CXX_STANDARD 17)

# Добавляем файлы с функциями
add_library(functions functions.cpp)

# Добавляем тесты
enable_testing()
add_executable(test_functions test_functions.cpp)
target_link_libraries(test_functions functions)
add_test(NAME testGetCurrentDir COMMAND test_functions testGetCurrentDir)
add_test(NAME testLogMessage COMMAND test_functions testLogMessage)
