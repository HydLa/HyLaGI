add_library(hydla_parser STATIC)

target_sources(hydla_parser
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/BuiltInParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ConstraintParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ExpressionParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/FunctionParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HydLaParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ListExpander.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ListParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParseTree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParseTreeGraphvizDumper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParseTreeJsonDumper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ParseTreeSemanticAnalyzer.cpp
)