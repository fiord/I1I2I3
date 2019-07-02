cmake_minimum_required(VERSION 2.8)
project(phone)
set(CMAKE_CXX_STANDARD 14)
add_executable(phone
  phone.cpp
  log.cpp
  connect.cpp
  video.cpp
  voice.cpp
)
find_package(OpenCV REQUIRED)
target_link_libraries(phone ${OpenCV_LIBS})
find_package(Threads REQUIRED)
target_link_libraries(phone Threads::Threads)
target_link_libraries(phone ncurses)
