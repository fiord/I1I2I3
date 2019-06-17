cmake_minimum_required(VERSION 2.8)
project(test)
set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV REQUIRED)
add_executable(test
  test_img.cpp
)
target_link_libraries(test ${OpenCV_LIBS})
