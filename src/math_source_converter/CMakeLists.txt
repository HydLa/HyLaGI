add_executable(msc)

target_sources(msc
    PRIVATE
    main.cpp
)

# set output directory
set_target_properties(msc
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)