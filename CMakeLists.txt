cmake_minimum_required(VERSION 2.8)

project(dataflow)

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAG} -std=c++11")
endif()

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAG} /EHsc")
endif()

add_subdirectory(cpp)
