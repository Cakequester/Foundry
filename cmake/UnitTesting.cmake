macro(initialize_cct_unittesting)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)

    add_library(cct_unittest INTERFACE)

    target_link_libraries(cct_unittest
        INTERFACE
            GTest::gtest_main
    )
endmacro()

function(add_cct_unittest name)
    cmake_parse_arguments(
        TEST
        ""
        ""
        "SOURCES;INCLUDE;LINK_LIBS"
        ${ARGN}
    )

    add_executable(${name}
        ${TEST_SOURCES}
    )

    target_compile_features(${name} PRIVATE
        c_std_99
        cxx_std_23
    )

    target_include_directories(${name}
        PRIVATE
            ${TEST_INCLUDE}
    )

    target_link_libraries(${name}
        PRIVATE
            cct_unittest
            ${TEST_LINK_LIBS}
    )

    include(GoogleTest)
    gtest_discover_tests(${name})
endfunction()