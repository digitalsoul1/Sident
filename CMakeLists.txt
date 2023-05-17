cmake_minimum_required(VERSION 3.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_LOG_DYN_LINK -pthread")
project(sidentd)

find_package(Boost 1.58 REQUIRED COMPONENTS system regex log program_options)
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cpp")

add_executable(sidentd ${SOURCES})

set_property(TARGET sidentd PROPERTY CXX_STANDARD 11)
target_link_libraries(sidentd ${Boost_LIBRARIES})
target_compile_options(sidentd PRIVATE -Werror)
