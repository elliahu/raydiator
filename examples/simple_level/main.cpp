#include <cmath>
#include <iostream>
#include <vector>
#include <raylib.h>

#include <raydiator.h>

using namespace raydiator;


#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

int main() {
    Window window = Window().create("Simple Level", SCREEN_WIDTH, SCREEN_HEIGHT);


    Level level;
    level.load("../../../data/levels/basic_level");



    Player player{
        .position = {level.start_pos_x, level.start_pos_y},
        .direction = {level.start_dir_x, level.start_dir_y},
        .plane = {-level.start_dir_y * level.plane_width, level.start_dir_x * level.plane_width},
    };

    LevelRenderer renderer{level, player, window};


    // Main game loop
    while (!window.shouldClose()) {
        renderer.update();


        renderer.draw();
    }

    window.close();

    return 0;
}
