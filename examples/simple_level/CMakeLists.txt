# Add the executable
add_executable(simple_level
        main.cpp
)

# Link the engine library
target_link_libraries(simple_level PRIVATE raydiator)
target_include_directories(simple_level PRIVATE ${PROJECT_SOURCE_DIR}/include)