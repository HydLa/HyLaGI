add_executable(hylagi)

target_sources(hylagi
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ProgramOptions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/symbolic_simulate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
)

target_include_directories(hylagi
    PRIVATE
    ${CMAKE_HOME_DIRECTORY}/src/backend/mathematica
)

target_link_libraries(hylagi PUBLIC ${Boost_LIBRARIES})

target_link_libraries(hylagi PUBLIC hydla_backend)
target_link_libraries(hylagi PUBLIC hydla_common)
target_link_libraries(hylagi PUBLIC hydla_debug)
target_link_libraries(hylagi PUBLIC hydla_hierarchy)
target_link_libraries(hylagi PUBLIC hydla_interval)
target_link_libraries(hylagi PUBLIC hydla_io)
target_link_libraries(hylagi PUBLIC hydla_parser)
target_link_libraries(hylagi PUBLIC hydla_simulator)
target_link_libraries(hylagi PUBLIC hydla_utility)

execute_process(
    COMMAND
    git show -s --format=%h
    RESULT_VARIABLE
    SHORT_HASH_RESULT
    OUTPUT_VARIABLE
    SHORT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
    COMMAND
    git rev-parse --abbrev-ref HEAD
    RESULT_VARIABLE
    BRANCH_NAME_RESULT
    OUTPUT_VARIABLE
    BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE)

target_compile_definitions(hylagi PRIVATE BRANCH_NAME="${BRANCH_NAME}")
target_compile_definitions(hylagi PRIVATE COMMIT_ID="${SHORT_HASH}")