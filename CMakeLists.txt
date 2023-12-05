cmake_minimum_required(VERSION 3.8)
project(zenoh_leak)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(zenohc REQUIRED)

add_executable(zenoh-leak
  zenoh-leak.cpp
)
target_link_libraries(zenoh-leak PRIVATE
  zenohc::lib
)
