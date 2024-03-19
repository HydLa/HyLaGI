add_library(hydla_interval STATIC)

target_sources(hydla_interval
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/AffineApproximator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AffineMixedValue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AffineOrInteger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AffineTreeVisitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IntervalNewton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IntervalOrInteger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IntervalTreeVisitor.cpp
)

# -lm -lrt
target_link_libraries(hydla_interval PRIVATE m)

target_compile_definitions(hydla_interval
    PRIVATE
    AFFINE_SIMPLE=1
)
