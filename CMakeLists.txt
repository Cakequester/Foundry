cmake_minimum_required (VERSION 4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(Foundry)

set(PROJECT_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")

if(BUILD_TESTING)
	add_subdirectory(unittests)
endif()