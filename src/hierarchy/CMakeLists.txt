add_library(hydla_hierarchy STATIC)

target_sources(hydla_hierarchy
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/IncrementalModuleSet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ModuleSet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ModuleSetContainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ModuleSetGraph.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ModuleSetList.cpp
)