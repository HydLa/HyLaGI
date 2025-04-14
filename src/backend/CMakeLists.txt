add_library(hydla_backend STATIC)

set(math_source ${CMAKE_CURRENT_SOURCE_DIR}/mathematica/math_source.cpp)

target_sources(hydla_backend
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mathematica/MathematicaLink.cpp

    ${math_source}
)

target_include_directories(hydla_backend PUBLIC
    ${Mathematica_WSTP_INCLUDE_DIR}
)

target_link_libraries(hydla_backend PUBLIC
    ${Mathematica_WSTP_LIBRARY}
)

add_custom_command(
    OUTPUT
    ${math_source}
    COMMAND
    cat ${CMAKE_CURRENT_SOURCE_DIR}/mathematica/math_source/load_first/*.m ${CMAKE_CURRENT_SOURCE_DIR}/mathematica/math_source/*.m | ${CMAKE_BINARY_DIR}/bin/msc math_source > ${CMAKE_CURRENT_SOURCE_DIR}/mathematica/math_source.cpp
    DEPENDS msc
)