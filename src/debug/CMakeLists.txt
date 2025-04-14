add_library(hydla_debug STATIC)

target_sources(hydla_debug
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/boost_algo_str.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/debug_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/json_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/solve_sym.cpp
)

target_compile_definitions(hydla_debug
    PRIVATE
    AFFINE_SIMPLE=1
)

target_include_directories(hydla_debug
    PUBLIC
    ${Python3_INCLUDE_DIRS}
)
target_link_libraries(hydla_debug PRIVATE Python3::Python)