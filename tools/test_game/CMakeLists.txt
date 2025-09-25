# Add the executable
add_executable(test_game
        main.cpp
)

# Link the engine library
target_link_libraries(test_game PRIVATE raydiator)
target_include_directories(test_game PRIVATE ${PROJECT_SOURCE_DIR})