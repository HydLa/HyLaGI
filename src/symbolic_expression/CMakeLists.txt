add_library(hydla_symbolic_expression STATIC)

target_sources(hydla_symbolic_expression
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/BaseNodeVisitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DefaultTreeVisitor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Node.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TreeInfixPrinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TreeVisitorForAtomicConstraint.cpp
)