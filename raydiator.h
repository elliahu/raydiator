#pragma once
#include <cfloat>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <raylib.h>
#include <filesystem>
#include <unordered_map>

#define TEX_WIDTH 64


namespace raydiator {
    namespace AssertUtils {
        // Behavior options for failed assertions
        enum class AssertAction {
            Abort,
            Throw,
            Log,
        };

        // Current action; can be set dynamically
        inline AssertAction CurrentAction = AssertAction::Abort;

        // Assert handler
        inline void HandleAssert(const char *expr, const char *file, int line, const char *func,
                                 const std::string &message) {
            // Construct the debug message
            std::string debugMessage = "[ASSERT FAILED]\n";
            debugMessage += "Expression: " + std::string(expr) + "\n";
            debugMessage += "File: " + std::string(file) + "\n";
            debugMessage += "Line: " + std::to_string(line) + "\n";
            debugMessage += "Function: " + std::string(func) + "\n";
            if (!message.empty()) {
                debugMessage += "Message: " + message + "\n";
            }

            switch (CurrentAction) {
                case AssertAction::Abort:
                    std::cerr << debugMessage << std::endl;
                    std::abort();
                case AssertAction::Throw:
                    throw std::runtime_error(debugMessage);
                case AssertAction::Log:
                    // Replace with your logging system if needed
                    std::cerr << debugMessage << std::endl;
                    break;
            }
        }
    } // namespace AssertUtils

    // Custom assert macro
#ifndef ASSERT
#define ASSERT(expr, message)                                            \
    do {                                                                 \
        if (!(expr)) {                                                   \
            AssertUtils::HandleAssert(#expr, __FILE__, __LINE__,         \
                                      __PRETTY_FUNCTION__, message);     \
        }                                                                \
    } while (false)
#endif

    // typedef representing single level map in game
    typedef std::vector<std::vector<unsigned char> > Map;
    typedef std::unordered_map<std::string, std::string> Metadata;

    /**
     * Player values and helpers
     */
    struct Player {
        Vector2 position;
        Vector2 direction;
        Vector2 plane;
        float movement_speed = 3.0f, look_speed = 2.5f;
    };

    /**
     * Type representing a single level in game
     */
    struct Level {
        // Map of the level
        Map map;
        Metadata metadata;

        float start_pos_x;
        float start_pos_y;
        float start_dir_x;
        float start_dir_y;
        float plane_width;
        int floor_style;
        int roof_style;

        Image atlas;

        /**
         * Returns map value at coord p
         * @param p Point to sample
         * @return value at a coord p
         */
        unsigned char at(Vector2 p) { return map[p.y][p.x]; }

        /**
         * Read metadata from loaded level
         * @tparam T Type of the value
         * @param key Key of the value
         * @return If parsed successfully, returns value of type T that corresponds to key key
         */
        template<typename T>
        T readMetadata(std::string key) {
            ASSERT(metadata.contains(key), "Metadata key not found");
            const std::string &value = metadata[key];
            std::istringstream stream(value);
            T result;

            // Attempt to convert the string to the requested type
            stream >> result;
            if (stream.fail() || !stream.eof()) {
                throw std::runtime_error("Failed to convert metadata value for key: " + key + " to the requested type");
            }

            return result;
        }

        /**
         * Loads a map from a file
         * @param filename name of the file where the map is stored
         */
        void load(std::string filename) {
            map.clear(); // Clear existing map data
            metadata.clear(); // Clear existing metadata

            std::ifstream file(filename);
            ASSERT(file.is_open(), "Could not open the level file");


            std::string line;

            ASSERT(std::getline(file, line) && line == "---", "Metadata block must start with '---'");

            while (std::getline(file, line)) {
                if (line == "---") break; // End of metadata block

                std::istringstream lineStream(line);
                std::string key, value;

                // Parse "key: value" pairs
                if (std::getline(lineStream, key, ':') && std::getline(lineStream >> std::ws, value)) {
                    metadata[key] = value;
                } else {
                    throw std::runtime_error("Invalid metadata format: " + line);
                }
            }

            ASSERT(line == "---", "Metadata block must end with '---'");

            // Parse map data
            while (std::getline(file, line)) {
                std::istringstream lineStream(line);
                std::vector<unsigned char> row;
                int cell;

                while (lineStream >> cell) {
                    ASSERT(cell >= 0 && cell <= 255, "Cell value out of range (0-255)");
                    row.push_back(static_cast<unsigned char>(cell));
                }

                if (!row.empty()) {
                    map.push_back(row);
                }
            }

            file.close();

            start_pos_x = readMetadata<float>("start_pos_x");
            start_pos_y = readMetadata<float>("start_pos_y");
            start_dir_x = readMetadata<float>("start_dir_x");
            start_dir_y = readMetadata<float>("start_dir_y");
            plane_width = readMetadata<float>("plane_width");
            floor_style = readMetadata<float>("floor_style");
            roof_style = readMetadata<float>("roof_style");

            // Read the atlas filename
            std::string atlas_filename = readMetadata<std::string>("atlas_filename");

            // Update atlas_filename to include the parent directory path from filename of map
            std::filesystem::path map_path(filename);  // Get the path object for the map file
            std::filesystem::path atlas_path = map_path.parent_path() / atlas_filename;  // Combine parent path with the atlas filename

            // Now atlas_filename includes the full path relative to the map's directory
            atlas = LoadImage(atlas_path.c_str());
        }
    };

    /**
     * Represents simple game window
     */
    struct Window {
        uint32_t width, height, target_fps;
        bool resizable, fullscreen;
        std::string title;

        /**
         * Create a game window
         * @param title Window title
         * @param width window width (horizontal size)
         * @param height window height (vertical size)
         * @param target_fps target fps cap
         * @param resizable is window resizable
         * @param fullscreen is window in fullscreen mode
         * @return returns window reference
         */
        Window &create(const std::string &title, uint32_t width, uint32_t height, uint32_t target_fps = 60,
                       bool resizable = true, bool fullscreen = false) {
            this->title = title;
            this->width = width;
            this->height = height;
            this->resizable = resizable;
            this->fullscreen = fullscreen;
            this->target_fps = 60;

            SetTraceLogLevel(TraceLogLevel::LOG_ERROR);
            InitWindow(static_cast<int>(width), static_cast<int>(height), title.c_str());
            SetTargetFPS(target_fps);

            // TODO fullscreen

            return *this;
        }

        /**
         * Check if window should be closed
         * @return True if close request was sent
         */
        bool shouldClose() {
            return WindowShouldClose();
        }

        /**
         * Close the window
         */
        void close() {
            CloseWindow();
        }
    };

    /**
     * Screen stages
     */
    typedef enum GameScreen { LOGO = 0, TITLE, GAMEPLAY, ENDING } GameScreen;

    /**
     * Base renderer
     */
    struct BaseRenderer {
        float time = 0.f, prev_time = 0.f, frame_time = 0.f;

        void update() {
            prev_time = time;
            time = GetTime();
            frame_time = (time - prev_time);
        }
    };

    inline Color mapValToColor(unsigned char value) {
        switch (value) {
            case 1: return GRAY;
            case 2: return WHITE;
            case 3: return DARKGRAY;
            case 4: return DARKPURPLE;
            default: return RED;
        }
    }

    inline Color darkenColor(Color color, float factor) {
        return Color(color.r * factor, color.g * factor, color.b * factor, color.a);
    }

    inline double normalize(float value, float min = 0, float max = 10) {
        ASSERT(min != max, "Main and max are the same");
        if (value < min) { value = min; }
        if (value > max) { value = max; }
        return 1 - (value - min) / (max - min);
    }

    /**
     * Renderer of levels
     */
    struct LevelRenderer : BaseRenderer {
        Level &level;
        Player &player;
        Window &window;
        Color bg_color = Color{0, 0, 0, 1};
        Image screen_buffer_image;
        Texture2D screen_buffer_texture;

        LevelRenderer(Level &level, Player &player, Window &window): level(level), player(player), window(window) {
            screen_buffer_image = GenImageColor(window.width, window.height, BLANK);
            screen_buffer_texture = LoadTextureFromImage(screen_buffer_image);
        }

        ~LevelRenderer() {
            UnloadImage(screen_buffer_image);
            UnloadTexture(screen_buffer_texture);
        }

        void update() {
            // this updates frame timing
            BaseRenderer::update();

            if (IsKeyDown(KEY_W)) {
                // Calculate future positions
                float futureX = player.position.x + player.direction.x * player.movement_speed * frame_time;
                float futureY = player.position.y + player.direction.y * player.movement_speed * frame_time;

                // Check collision for X and Y separately
                if (level.map[(int) player.position.y][(int) futureX] == 0) // Check horizontal collision
                    player.position.x = futureX;
                if (level.map[(int) futureY][(int) player.position.x] == 0) // Check vertical collision
                    player.position.y = futureY;
            }

            if (IsKeyDown(KEY_S)) {
                float futureX = player.position.x - player.direction.x * player.movement_speed * frame_time;
                float futureY = player.position.y - player.direction.y * player.movement_speed * frame_time;

                if (level.map[(int) player.position.y][(int) futureX] == 0)
                    player.position.x = futureX;
                if (level.map[(int) futureY][(int) player.position.x] == 0)
                    player.position.y = futureY;
            }

            if (IsKeyDown(KEY_D)) {
                // Strafe left
                float strafeX = -player.direction.y;
                float strafeY = player.direction.x;
                float futureX = player.position.x + strafeX * player.movement_speed * frame_time;
                float futureY = player.position.y + strafeY * player.movement_speed * frame_time;

                if (level.map[(int) player.position.y][(int) futureX] == 0)
                    player.position.x = futureX;
                if (level.map[(int) futureY][(int) player.position.x] == 0)
                    player.position.y = futureY;
            }

            if (IsKeyDown(KEY_A)) {
                // Strafe right
                float strafeX = player.direction.y;
                float strafeY = -player.direction.x;
                float futureX = player.position.x + strafeX * player.movement_speed * frame_time;
                float futureY = player.position.y + strafeY * player.movement_speed * frame_time;

                if (level.map[(int) player.position.y][(int) futureX] == 0)
                    player.position.x = futureX;
                if (level.map[(int) futureY][(int) player.position.x] == 0)
                    player.position.y = futureY;
            }

            // Rotation
            if (IsKeyDown(KEY_LEFT)) {
                // Rotate right
                float oldDirX = player.direction.x;
                player.direction.x = player.direction.x * cos(-player.look_speed * frame_time) - player.direction.y *
                                     sin(
                                         -player.look_speed * frame_time);
                player.direction.y = oldDirX * sin(-player.look_speed * frame_time) + player.direction.y * cos(
                                         -player.look_speed * frame_time);
                float oldPlaneX = player.plane.x;
                player.plane.x = player.plane.x * cos(-player.look_speed * frame_time) - player.plane.y * sin(
                                     -player.look_speed * frame_time);
                player.plane.y = oldPlaneX * sin(-player.look_speed * frame_time) + player.plane.y * cos(
                                     -player.look_speed * frame_time);
            }
            if (IsKeyDown(KEY_RIGHT)) {
                // Rotate left
                float oldDirX = player.direction.x;
                player.direction.x = player.direction.x * cos(player.look_speed * frame_time) - player.direction.y *
                                     sin(
                                         player.look_speed * frame_time);
                player.direction.y = oldDirX * sin(player.look_speed * frame_time) + player.direction.y * cos(
                                         player.look_speed * frame_time);
                float oldPlaneX = player.plane.x;
                player.plane.x = player.plane.x * cos(player.look_speed * frame_time) - player.plane.y * sin(
                                     player.look_speed * frame_time);
                player.plane.y = oldPlaneX * sin(player.look_speed * frame_time) + player.plane.y * cos(
                                     player.look_speed * frame_time);
            }
        }

        void draw() {
            BeginDrawing();
            ClearBackground(bg_color);

            for (int x = 0; x < window.width; x++) {
                //calculate ray position and direction
                float camera_x = 2 * x / static_cast<float>(window.width) - 1;
                Vector2 ray_dir = {
                    player.direction.x + player.plane.x * camera_x,
                    player.direction.y + player.plane.y * camera_x
                };

                //which box of the map we're in
                Vector2 map{floor(player.position.x), floor(player.position.y)}; // Added floor() for safety

                //length of ray from current position to next x or y-side
                Vector2 side_dist;

                //length of ray from one x or y-side to next x or y-side
                Vector2 delta_dist{
                    (ray_dir.x == 0) ? 1e30f : std::abs(1 / ray_dir.x),
                    (ray_dir.y == 0) ? 1e30f : std::abs(1 / ray_dir.y)
                };

                //what direction to step in x or y-direction (either +1 or -1)
                Vector2 dda_step;

                int hit = 0; //was there a wall hit?
                int side; //was a NS or a EW wall hit?

                //calculate step and initial sideDist
                if (ray_dir.x < 0) {
                    dda_step.x = -1;
                    side_dist.x = (player.position.x - map.x) * delta_dist.x;
                } else {
                    dda_step.x = 1;
                    side_dist.x = (map.x + 1.0 - player.position.x) * delta_dist.x;
                }

                if (ray_dir.y < 0) {
                    dda_step.y = -1;
                    side_dist.y = (player.position.y - map.y) * delta_dist.y;
                } else {
                    dda_step.y = 1;
                    side_dist.y = (map.y + 1.0 - player.position.y) * delta_dist.y;
                }

                //perform DDA
                while (hit == 0) {
                    //jump to next map square, either in x-direction, or in y-direction
                    if (side_dist.x < side_dist.y) {
                        side_dist.x += delta_dist.x;
                        map.x += dda_step.x;
                        side = 0;
                    } else {
                        side_dist.y += delta_dist.y;
                        map.y += dda_step.y;
                        side = 1;
                    }
                    //Check if ray has hit a wall
                    if (level.at(map) > 0) hit = 1;
                }

                //Calculate distance projected on camera direction
                double perp_wall_dist;
                if (side == 0) perp_wall_dist = (side_dist.x - delta_dist.x);
                else perp_wall_dist = (side_dist.y - delta_dist.y);

                // Prevent division by zero or negative distances
                if (perp_wall_dist <= 0) perp_wall_dist = 0.001;

                //Calculate height of line to draw on screen
                int wall_height = (int) (window.height / perp_wall_dist);

                //calculate lowest and highest pixel to fill in current stripe
                int wall_start = -wall_height / 2 + window.height / 2;
                if (wall_start < 0) wall_start = 0;
                int wall_end = wall_height / 2 + window.height / 2;
                if (wall_end >= window.height) wall_end = window.height - 1;

                // Get texture number from map (make sure it's valid)
                int texNum = level.at(map) - 1;
                if (texNum < 0) texNum = 0;

                // Calculate wall X (where the ray hit the wall)
                float wallX;
                if (side == 0) {
                    wallX = player.position.y + perp_wall_dist * ray_dir.y;
                } else {
                    wallX = player.position.x + perp_wall_dist * ray_dir.x;
                }
                wallX -= floor(wallX);

                // Calculate texture coordinates
                int texX = (int) (wallX * level.atlas.height);

                // Flip texture coordinates if needed
                if (side == 0 && ray_dir.x > 0) texX = level.atlas.height - texX - 1;
                if (side == 1 && ray_dir.y < 0) texX = level.atlas.height - texX - 1;

                // Calculate final texture X coordinate including offset for texture number
                texX = abs(texX) + (texNum * level.atlas.height);

                // Make sure texX is within bounds
                texX = std::max(0, std::min(texX, level.atlas.width - 1));

                // Calculate texture step and starting position
                float _step = (float) level.atlas.height / wall_height;
                float texPos = (wall_start - window.height / 2 + wall_height / 2) * _step;

                // Draw the vertical stripe
                for (int y = wall_start; y < wall_end; y++) {
                    int texY = (int) texPos & (level.atlas.height - 1);
                    texPos += _step;

                    // Ensure texture coordinates are valid
                    texY = std::max(0, std::min(texY, level.atlas.height - 1));

                    // Get color from texture
                    Color color = GetImageColor(level.atlas, texX, texY);


                    // Apply shading based on distance and side
                    if (side == 1) {
                        color.r = color.r * 0.7;
                        color.g = color.g * 0.7;
                        color.b = color.b * 0.7;
                    }

                    // Draw pixel
                    ImageDrawPixel(&screen_buffer_image, x, y, color);
                }
            }

            // Draw the frame
            UpdateTexture(screen_buffer_texture, screen_buffer_image.data);
            DrawTexture(screen_buffer_texture, 0, 0, WHITE);


            // Draw debug info
            DrawText(TextFormat("FPS: %d", GetFPS()),10, 10, 20, WHITE);
            DrawText(TextFormat("Pos: %.2f, %.2f", player.position.x, player.position.y), 10, 30, 20, WHITE);
            DrawText(TextFormat("Dir: %.2f, %.2f", player.direction.x, player.direction.y), 10, 50, 20, WHITE);

            ImageClearBackground(&screen_buffer_image, Color{0, 0, 0, 0});


            EndDrawing();
        }
    };
}
