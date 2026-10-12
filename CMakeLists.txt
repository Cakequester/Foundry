cmake_minimum_required (VERSION 4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(Foundry)

set(CCT_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(unittests)