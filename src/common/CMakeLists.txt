add_library(hydla_common STATIC)

target_sources(hydla_common
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SignalHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Timer.cpp
)