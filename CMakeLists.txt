cmake_minimum_required(VERSION 3.5)

project(test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_executable(test main.cpp)
find_package(Threads REQUIRED)
target_link_libraries(test PRIVATE Threads::Threads)
