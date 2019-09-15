cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)

project(bowshock_server)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

find_path(BEAST_INCLUDE_DIR boost/beast/core.hpp)
find_path(ASIO_INCLUDE_DIR boost/asio/strand.hpp)

find_package(nlohmann_json CONFIG REQUIRED)

find_library(ASIO_LIBRARY lib-asio)

add_subdirectory(src)
