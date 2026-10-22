add_unittest(perfect_map
    SOURCES
        perfect_map.cpp
    INCLUDE
        ${CCT_INCLUDE_PATH}
)

add_unittest(perfect_set
    SOURCES
        perfect_set.cpp
    INCLUDE
        ${CCT_INCLUDE_PATH}
)

add_unittest(integral
    SOURCES
        integral.cpp
    INCLUDE
        ${CCT_INCLUDE_PATH}
)

add_unittest(type_list
    SOURCES
        type_list.cpp
    INCLUDE
        ${CCT_INCLUDE_PATH}
)

add_unittest(value_list
    SOURCES
        value_list.cpp
    INCLUDE
        ${CCT_INCLUDE_PATH}
)

add_subdirectory(meta)