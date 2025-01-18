#include <cmath>
#include <iostream>
#include <vector>
#include <raylib.h>


#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080


typedef std::vector<std::vector<unsigned char> > Map;

Map map =
{
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 3, 0, 3, 0, 3, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2, 0, 0, 0, 0, 3, 0, 3, 0, 3, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 4, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 4, 0, 0, 0, 0, 5, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 4, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 4, 0, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};


struct Player {
    Vector2 position;
    Vector2 direction;
    Vector2 plane;
    float time, prev_time;
};

struct Level {
    Map map;
    unsigned char at(Vector2 p) { return map[p.y][p.x]; }
};

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "toaster");
    SetTargetFPS(60);

    Player player{
        .position = {22.0f, 12.0f},
        .direction = {-1.0f, 0.0f},
        .plane = {0.f, 0.66f},
        .time = 0,
        .prev_time = 0,
    };
    Level level;
    level.map = map;


    // Main game loop
    while (!WindowShouldClose()) {
        // Timing for frame-independent movement

        player.prev_time = player.time;
        player.time = GetTime();
        float frameTime = (player.time - player.prev_time);

        // Movement speed constants
        const float moveSpeed =  10.0f * frameTime;
        const float rotSpeed = 3.f * frameTime;

        if (IsKeyDown(KEY_W)) {
            // Calculate future positions
            float futureX = player.position.x + player.direction.x * moveSpeed;
            float futureY = player.position.y + player.direction.y * moveSpeed;

            // Check collision for X and Y separately
            if (level.map[(int)player.position.y][(int)futureX] == 0) // Check horizontal collision
                player.position.x = futureX;
            if (level.map[(int)futureY][(int)player.position.x] == 0) // Check vertical collision
                player.position.y = futureY;
        }

        if (IsKeyDown(KEY_S)) {
            float futureX = player.position.x - player.direction.x * moveSpeed;
            float futureY = player.position.y - player.direction.y * moveSpeed;

            if (level.map[(int)player.position.y][(int)futureX] == 0)
                player.position.x = futureX;
            if (level.map[(int)futureY][(int)player.position.x] == 0)
                player.position.y = futureY;
        }

        if (IsKeyDown(KEY_A)) {
            // Strafe left
            float strafeX = -player.direction.y;
            float strafeY = player.direction.x;
            float futureX = player.position.x + strafeX * moveSpeed;
            float futureY = player.position.y + strafeY * moveSpeed;

            if (level.map[(int)player.position.y][(int)futureX] == 0)
                player.position.x = futureX;
            if (level.map[(int)futureY][(int)player.position.x] == 0)
                player.position.y = futureY;
        }

        if (IsKeyDown(KEY_D)) {
            // Strafe right
            float strafeX = player.direction.y;
            float strafeY = -player.direction.x;
            float futureX = player.position.x + strafeX * moveSpeed;
            float futureY = player.position.y + strafeY * moveSpeed;

            if (level.map[(int)player.position.y][(int)futureX] == 0)
                player.position.x = futureX;
            if (level.map[(int)futureY][(int)player.position.x] == 0)
                player.position.y = futureY;
        }

        // Rotation
        if (IsKeyDown(KEY_RIGHT)) {
            // Rotate right
            float oldDirX = player.direction.x;
            player.direction.x = player.direction.x * cos(-rotSpeed) - player.direction.y * sin(-rotSpeed);
            player.direction.y = oldDirX * sin(-rotSpeed) + player.direction.y * cos(-rotSpeed);
            float oldPlaneX = player.plane.x;
            player.plane.x = player.plane.x * cos(-rotSpeed) - player.plane.y * sin(-rotSpeed);
            player.plane.y = oldPlaneX * sin(-rotSpeed) + player.plane.y * cos(-rotSpeed);
        }
        if (IsKeyDown(KEY_LEFT)) {
            // Rotate left
            float oldDirX = player.direction.x;
            player.direction.x = player.direction.x * cos(rotSpeed) - player.direction.y * sin(rotSpeed);
            player.direction.y = oldDirX * sin(rotSpeed) + player.direction.y * cos(rotSpeed);
            float oldPlaneX = player.plane.x;
            player.plane.x = player.plane.x * cos(rotSpeed) - player.plane.y * sin(rotSpeed);
            player.plane.y = oldPlaneX * sin(rotSpeed) + player.plane.y * cos(rotSpeed);
        }


        BeginDrawing();
        ClearBackground(Color{0, 0, 0, 1});

        for (int x = 0; x < SCREEN_WIDTH; x++) {
            //calculate ray position and direction
            float camera_x = 2 * x / static_cast<float>(SCREEN_WIDTH) - 1;
            Vector2 ray_dir = {
                player.direction.x + player.plane.x * camera_x,
                player.direction.y + player.plane.y * camera_x
            };

            //which box of the map we're in
            Vector2 map{player.position.x, player.position.y};

            //length of ray from current position to next x or y-side
            Vector2 side_dist;

            //length of ray from one x or y-side to next x or y-side
            Vector2 delta_dist{std::abs(1 / ray_dir.x), std::abs(1 / ray_dir.y)};


            //what direction to step in x or y-direction (either +1 or -1)
            Vector2 step;

            int hit = 0; //was there a wall hit?
            int side; //was a NS or a EW wall hit?

            //calculate step and initial sideDist
            if (ray_dir.x < 0) {
                step.x = -1;
                side_dist.x = (player.position.x - floor(player.position.x)) * delta_dist.x;
            } else {
                step.x = 1;
                side_dist.x = (ceil(player.position.x) - player.position.x) * delta_dist.x;
            }

            if (ray_dir.y < 0) {
                step.y = -1;
                side_dist.y = (player.position.y - floor(player.position.y)) * delta_dist.y;
            } else {
                step.y = 1;
                side_dist.y = (ceil(player.position.y) - player.position.y) * delta_dist.y;
            }

            //perform DDA
            while (hit == 0) {
                //jump to next map square, either in x-direction, or in y-direction
                if (side_dist.x < side_dist.y) {
                    side_dist.x += delta_dist.x;
                    map.x += step.x;
                    side = 0;
                } else {
                    side_dist.y += delta_dist.y;
                    map.y += step.y;
                    side = 1;
                }
                //Check if ray has hit a wall
                if (level.at(map) > 0) hit = 1;
            }

            //Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
            double perp_wall_dist;
            if (side == 0) perp_wall_dist = (side_dist.x - delta_dist.x);
            else perp_wall_dist = (side_dist.y - delta_dist.y);

            //Calculate height of line to draw on screen
            int lineHeight = (int) (SCREEN_HEIGHT / perp_wall_dist);

            //calculate lowest and highest pixel to fill in current stripe
            int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
            if (drawStart < 0)drawStart = 0;
            int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
            if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;
            Color color;
            switch (level.at(map)) {
                case 1: color = RED; break;
                case 2: color = BLUE; break;
                case 3: color = YELLOW; break;
                case 4: color = GREEN; break;
                default: color = WHITE; break;
            }

            //give x and y sides different brightness
            if (side == 1) {  // Darken y-sides
                color.r = color.r / 2;
                color.g = color.g / 2;
                color.b = color.b / 2;
            }

            //draw the pixels of the stripe as a vertical line
            DrawLine(x, drawStart, x, drawEnd, color );
        }



        EndDrawing();
    }

    CloseWindow();

    return 0;
}