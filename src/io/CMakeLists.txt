add_library(hydla_io STATIC)

target_sources(hydla_io
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Constants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CsvProfilePrinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MathematicaTrajPrinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StdProfilePrinter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SymbolicTrajPrinter.cpp
)