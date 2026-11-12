cmake_minimum_required (VERSION 4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(Foundry)

set(PROJECT_INCLUDE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(Foundry INTERFACE)

target_include_directories(Foundry INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(Foundry INTERFACE
    c_std_99
    cxx_std_23
)

if(BUILD_TESTING)
	add_subdirectory(unittests)
endif()