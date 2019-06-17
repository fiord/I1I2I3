cmake_minimum_required(VERSION 2.8)
project(phone)
set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV REQUIRED)
add_executable(phone
  phone.cpp
  die.cpp
  connect.cpp
  video.cpp
  voice.cpp
)
find_package(Threads REQUIRED)
target_link_libraries(phone Threads::Threads)
target_link_libraries(phone ${OpenCV_LIBS})
