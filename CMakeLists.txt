cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)

project ("JigChecker")

enable_testing()

add_library(Solver OBJECT ExactCover.cpp)

add_executable (JigChecker JigChecker.cpp $<TARGET_OBJECTS:Solver>)

add_executable(test_Sudoku test_Sudoku.cpp  $<TARGET_OBJECTS:Solver>)
add_test(NAME test_Sudoku COMMAND test_Sudoku)
