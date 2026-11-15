cmake_minimum_required (VERSION 4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(foundry)

set(PROJECT_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(foundry INTERFACE)

target_include_directories(foundry INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(foundry INTERFACE
    c_std_99
    cxx_std_23
)

if(BUILD_TESTING)
	add_subdirectory(unittests)
endif()