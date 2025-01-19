# Add the executable
add_executable(level_editor
        main.cpp
)

# Link the engine library
target_link_libraries(level_editor PRIVATE raydiator)
target_include_directories(level_editor PRIVATE ${PROJECT_SOURCE_DIR})