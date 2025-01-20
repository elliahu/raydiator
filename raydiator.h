#pragma once
#include <algorithm>
#include <array>
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


#define SPRITE_ATLAS_SIZE 512
#define SPRITE_TEXTURE_SIZE 32
#define SPRITE_TEXTURES_PER_ROW 16
#define SPRITE_WALL_ROW_IDX 0
#define SPRITE_CHEST_ROW_IDX 1
#define SPRITE_ENEMY_ROW_IDX 2

#define PLAYER_MAX_WEAPONS 4
#define PLAYER_MAX_KEYS 1
#define PLAYER_MAX_HP 10
#define PLAYER_MAX_SP 100


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
     * Item that player can use
     */
    struct Item {
        enum class Type {
            Empty,
            Weapon,
            Key
        } type;
        std::string name = "Empty";
    };

    struct Weapon : Item {
        unsigned int dmg;
    };

    struct Key : Item {
        enum class AccessLevel {
            FIRST, SECOND, THIRD
        } access_level;
    };

    /**
     * Player values and helpers
     */
    struct Player {
        Vector2 position;
        Vector2 direction;
        Vector2 plane;
        float movement_speed = 2.0f, look_speed = 1.5f;
        unsigned int hp = PLAYER_MAX_HP;
        unsigned int sp = 0;
        std::array<Weapon, PLAYER_MAX_WEAPONS> weapons{};
        std::array<Key, PLAYER_MAX_KEYS> keys{};
    };;

    /**
     * Sprite is a 2D image rendered in a scene, can be animated
     */
    struct Sprite {
        enum class Type {
            CHEST, ENEMY
        }type;
        Vector2 position; // Position in the world
        int texture_index; // Index in the sprite texture atlas
        float distance; // Distance from player (used for sorting)
    };

    /**
     * Chest sprite, opened and closed
     */
    struct Chest : Sprite {
        bool opened = false;
        int closed_offset = 0;
        int opened_offset = 1;
    };

    /**
     * Enemy sprite along with its animations
     */
    struct Enemy: Sprite {
        int idle_anim_offset;
        int idle_anime_stride;
    };

    // Helper function to get texture coordinates from index
    void getAtlasCoordinates(int textureIndex, int &x, int &y) {
        x = (textureIndex % SPRITE_TEXTURES_PER_ROW) * SPRITE_TEXTURE_SIZE;
        y = (textureIndex / SPRITE_TEXTURES_PER_ROW) * SPRITE_TEXTURE_SIZE;
    }


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
        std::vector<Sprite> sprites;

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

            // Read metadata
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
            std::filesystem::path map_path(filename); // Get the path object for the map file
            std::filesystem::path atlas_path = map_path.parent_path() / atlas_filename;
            // Combine parent path with the atlas filename

            // Now atlas_filename includes the full path relative to the map's directory
            atlas = LoadImage(atlas_path.c_str());

            // add sprites to the world
            for (int y = 0; y < map.size(); y++) {
                for (int x = 0; x < map[0].size(); x++) {
                    if (map[y][x] >= SPRITE_TEXTURES_PER_ROW * SPRITE_CHEST_ROW_IDX && map[y][x] < (SPRITE_TEXTURES_PER_ROW * SPRITE_CHEST_ROW_IDX + SPRITE_TEXTURES_PER_ROW)) {
                        Chest chest{};
                        chest.position = Vector2(x + 0.5f, y + 0.5f);
                        chest.distance = 0.0f;
                        chest.texture_index = map[y][x];
                        chest.closed_offset = 0;
                        chest.opened_offset = 1;
                        chest.type = Sprite::Type::CHEST;
                        sprites.push_back(chest);
                    }
                    if (map[y][x] >= SPRITE_TEXTURES_PER_ROW * SPRITE_ENEMY_ROW_IDX && map[y][x] < (SPRITE_TEXTURES_PER_ROW * SPRITE_ENEMY_ROW_IDX + SPRITE_TEXTURES_PER_ROW)) {
                        Enemy enemy{};
                        enemy.type = Sprite::Type::ENEMY;
                        enemy.position = Vector2(x + 0.5f, y + 0.5f);
                        enemy.distance = 0.0f;
                        enemy.texture_index = map[y][x];
                        enemy.idle_anim_offset = 0;
                        enemy.idle_anime_stride = 3;
                        sprites.push_back(enemy);
                    }
                }
            }
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

            //SetTraceLogLevel(TraceLogLevel::LOG_ERROR);
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

    enum class RayHit {
        NONE,
        WALL,
        SPRITE
    };

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
        uint32_t buffer_width, buffer_height;
        float buffer_scale_factor = 0.5;
        std::vector<Sprite> visible_sprites;
        float *z_buffer;
        int frame_counter = 0;
        int frame_speed = 6;
        int current_frame = 0;

        LevelRenderer(Level &level, Player &player, Window &window): level(level), player(player), window(window) {
            buffer_width = window.width * buffer_scale_factor;
            buffer_height = window.height * buffer_scale_factor;
            screen_buffer_image = GenImageColor(buffer_width, buffer_height, BLANK);
            screen_buffer_texture = LoadTextureFromImage(screen_buffer_image);
            z_buffer = new float[buffer_width];
        }

        ~LevelRenderer() {
            //UnloadTexture(screen_buffer_texture);
            UnloadImage(screen_buffer_image);
            delete[] z_buffer;
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

            frame_counter++;

            if (frame_counter >= (60/frame_speed))
            {
                frame_counter = 0;
                current_frame++;

                if (current_frame > 2) current_frame = 0;
            }
        }


        void draw() {
            BeginDrawing();
            ClearBackground(bg_color);

            // Clear z-buffer
            for (int x = 0; x < buffer_width; x++) {
                z_buffer[x] = std::numeric_limits<float>::infinity();
            }

            for (int x = 0; x < buffer_width; x++) {
                //calculate ray position and direction
                float camera_x = 2 * x / static_cast<float>(buffer_width) - 1;
                Vector2 ray_dir = {
                    player.direction.x + player.plane.x * camera_x,
                    player.direction.y + player.plane.y * camera_x
                };

                //which box of the map we're in
                Vector2 hit_box{floor(player.position.x), floor(player.position.y)}; // Added floor() for safety

                //length of ray from current position to next x or y-side
                Vector2 side_dist;

                //length of ray from one x or y-side to next x or y-side
                Vector2 delta_dist{
                    (ray_dir.x == 0) ? 1e30f : std::abs(1 / ray_dir.x),
                    (ray_dir.y == 0) ? 1e30f : std::abs(1 / ray_dir.y)
                };

                //what direction to step in x or y-direction (either +1 or -1)
                Vector2 dda_step;

                RayHit hit = RayHit::NONE; //was there a wall hit?
                int side; //was a NS or a EW wall hit?

                //calculate step and initial sideDist
                if (ray_dir.x < 0) {
                    dda_step.x = -1;
                    side_dist.x = (player.position.x - hit_box.x) * delta_dist.x;
                } else {
                    dda_step.x = 1;
                    side_dist.x = (hit_box.x + 1.0 - player.position.x) * delta_dist.x;
                }

                if (ray_dir.y < 0) {
                    dda_step.y = -1;
                    side_dist.y = (player.position.y - hit_box.y) * delta_dist.y;
                } else {
                    dda_step.y = 1;
                    side_dist.y = (hit_box.y + 1.0 - player.position.y) * delta_dist.y;
                }

                //perform DDA
                while (hit != RayHit::WALL) {
                    //jump to next map square, either in x-direction, or in y-direction
                    if (side_dist.x < side_dist.y) {
                        side_dist.x += delta_dist.x;
                        hit_box.x += dda_step.x;
                        side = 0;
                    } else {
                        side_dist.y += delta_dist.y;
                        hit_box.y += dda_step.y;
                        side = 1;
                    }
                    //Check if ray has hit a wall
                    if (level.at(hit_box) > SPRITE_TEXTURES_PER_ROW *  SPRITE_WALL_ROW_IDX && level.at(hit_box) < SPRITE_TEXTURES_PER_ROW *  SPRITE_WALL_ROW_IDX + SPRITE_TEXTURES_PER_ROW) hit = RayHit::WALL;
                }


                //Calculate distance projected on camera direction
                float perp_hit_distance;
                if (side == 0) perp_hit_distance = (side_dist.x - delta_dist.x);
                else perp_hit_distance = (side_dist.y - delta_dist.y);

                // Prevent division by zero or negative distances
                if (perp_hit_distance <= 0) perp_hit_distance = 0.01;

                // Store the perpendicular distance in the z-buffer
                z_buffer[x] = perp_hit_distance;

                //Calculate height of line to draw on screen
                int hit_object_height = (int) (buffer_height / perp_hit_distance);

                //calculate lowest and highest pixel to fill in current stripe
                int hit_object_y_min = -hit_object_height / 2 + buffer_height / 2;
                if (hit_object_y_min < 0) hit_object_y_min = 0;
                int hit_object_y_max = hit_object_height / 2 + buffer_height / 2;
                if (hit_object_y_max >= buffer_height) hit_object_y_max = buffer_height - 1;

                // Get texture number from map (make sure it's valid)
                int texNum = level.at(hit_box) - 1;

                // Calculate wall X (where the ray hit the wall)
                float wallX;
                if (side == 0) {
                    wallX = player.position.y + perp_hit_distance * ray_dir.y;
                } else {
                    wallX = player.position.x + perp_hit_distance * ray_dir.x;
                }
                wallX -= floor(wallX); // This gives us the fractional part (0-1)

                int baseTexX, baseTexY;
                getAtlasCoordinates(texNum, baseTexX, baseTexY);

                // Calculate the X offset within the 32x32 texture (using full precision)
                int texX = static_cast<int>(wallX * SPRITE_TEXTURE_SIZE);
                // Ensure it stays within the texture bounds
                texX = texX & (SPRITE_TEXTURE_SIZE - 1); // This is better than min/max for wrapping
                // Add the base coordinate for the atlas
                texX = baseTexX + texX;

                // Calculate texture step and starting position
                float _step = static_cast<float>(SPRITE_TEXTURE_SIZE) / hit_object_height;
                float texPos = (hit_object_y_min - buffer_height / 2 + hit_object_height / 2) * _step;

                // Draw the vertical stripe
                for (int y = hit_object_y_min; y < hit_object_y_max; y++) {
                    // Calculate Y coordinate within the 32x32 texture
                    int texY = static_cast<int>(texPos) & (SPRITE_TEXTURE_SIZE - 1);
                    texPos += _step;

                    // Add base Y coordinate for this texture in the atlas
                    texY = baseTexY + texY;

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

            // Prepare sprites for rendering
            visible_sprites.clear();
            for (const auto &sprite: level.sprites) {
                // Transform sprite position relative to camera
                float sprite_x = sprite.position.x - player.position.x;
                float sprite_y = sprite.position.y - player.position.y;

                // Transform sprite with the inverse camera matrix
                float inv_det = 1.0f / (player.plane.x * player.direction.y - player.direction.x * player.plane.y);
                float transform_x = inv_det * (player.direction.y * sprite_x - player.direction.x * sprite_y);
                float transform_y = inv_det * (-player.plane.y * sprite_x + player.plane.x * sprite_y);

                // Only add sprites that are in front of the camera
                if (transform_y > 0.1f) {
                    Sprite visible_sprite = sprite;
                    visible_sprite.distance = transform_y;
                    visible_sprites.push_back(visible_sprite);
                }
            }

            // Sort sprites from far to close
            std::sort(visible_sprites.begin(), visible_sprites.end(),
                      [](const Sprite &a, const Sprite &b) { return a.distance > b.distance; });

            // Render sprites
            for (const auto &sprite: visible_sprites) {
                // Transform sprite position relative to camera
                float sprite_x = sprite.position.x - player.position.x;
                float sprite_y = sprite.position.y - player.position.y;

                // Transform sprite with the inverse camera matrix
                float inv_det = 1.0f / (player.plane.x * player.direction.y - player.direction.x * player.plane.y);
                float transform_x = inv_det * (player.direction.y * sprite_x - player.direction.x * sprite_y);
                float transform_y = inv_det * (-player.plane.y * sprite_x + player.plane.x * sprite_y);

                // Early exit if behind camera
                if (transform_y <= 0.1f) continue;

                // Calculate sprite screen position
                int sprite_screen_x = int((buffer_width / 2) * (1.0f + transform_x / transform_y));

                // Calculate sprite dimensions on screen
                float sprite_scale = 1.0f; // Adjust this value to change sprite size
                int sprite_height = abs(int((buffer_height / transform_y) * sprite_scale));
                int sprite_width = sprite_height; // Keep aspect ratio 1:1

                // Calculate vertical drawing bounds
                int draw_start_y = -sprite_height / 2 + buffer_height / 2;
                int draw_end_y = sprite_height / 2 + buffer_height / 2;
                draw_start_y = std::max(0, draw_start_y);
                draw_end_y = std::min((int) buffer_height - 1, draw_end_y);

                // Calculate horizontal drawing bounds
                int draw_start_x = -sprite_width / 2 + sprite_screen_x;
                int draw_end_x = sprite_width / 2 + sprite_screen_x;
                draw_start_x = std::max(0, draw_start_x);
                draw_end_x = std::min((int) buffer_width - 1, draw_end_x);

                // Draw sprite
                for (int stripe = draw_start_x; stripe < draw_end_x; stripe++) {
                    if (transform_y < z_buffer[stripe]) {
                        // Depth test
                        // Calculate texture X coordinate
                        float fx = (stripe - (sprite_screen_x - sprite_width / 2)) / (float) sprite_width;
                        int texX = int(fx * SPRITE_TEXTURE_SIZE);
                        texX = std::clamp(texX, 0, SPRITE_TEXTURE_SIZE - 1); // Use clamp instead of wrap

                        // Get base coordinates for this sprite's texture
                        int baseTexX=0, baseTexY = 0;
                        if (sprite.type == Sprite::Type::CHEST) {
                            Chest chest = Chest(sprite);
                            getAtlasCoordinates(chest.texture_index + ((chest.opened)? chest.opened_offset : chest.closed_offset) , baseTexX, baseTexY);
                        }
                        else if (sprite.type == Sprite::Type::ENEMY) {
                            Enemy enemy = Enemy(sprite);
                            getAtlasCoordinates(sprite.texture_index + current_frame, baseTexX, baseTexY);
                        }

                        texX = baseTexX + texX;

                        // Vertical texture loop
                        for (int y = draw_start_y; y < draw_end_y; y++) {
                            // Calculate texture Y coordinate
                            float fy = (y - (buffer_height / 2 - sprite_height / 2)) / (float) sprite_height;
                            int texY = int(fy * SPRITE_TEXTURE_SIZE);
                            texY = std::clamp(texY, 0, SPRITE_TEXTURE_SIZE - 1); // Use clamp instead of wrap

                            texY = baseTexY + texY;

                            // Get color from texture atlas
                            Color color = GetImageColor(level.atlas, texX, texY);
                            if (color.a > 0) {
                                ImageDrawPixel(&screen_buffer_image, stripe, y, color);
                            }
                        }
                    }
                }
            }

            // Draw background gradient
            DrawRectangleGradientV(0, 0, window.width, window.height / 2, Color{176, 99, 16, 1}, BLACK);
            DrawRectangleGradientV(0, window.height / 2, window.width, window.height / 2, BLACK, Color{176, 99, 16, 1});

            // Draw the frame
            UpdateTexture(screen_buffer_texture, screen_buffer_image.data);
            DrawTexturePro(
                screen_buffer_texture, // The texture to draw
                Rectangle{0, 0, static_cast<float>(buffer_width), static_cast<float>(buffer_height)},
                // Source rectangle (flipped vertically)
                Rectangle{0, 0, static_cast<float>(window.width), static_cast<float>(window.height)},
                // Destination rectangle (scaled to full screen)
                Vector2{0, 0}, // Origin (top-left corner)
                0.0f, // Rotation (no rotation)
                WHITE // Tint (no tint, full color)
            );


            // Draw debug info
            DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, WHITE);
            DrawText(TextFormat("Pos: %.2f, %.2f", player.position.x, player.position.y), 10, 30, 20, WHITE);
            DrawText(TextFormat("Dir: %.2f, %.2f", player.direction.x, player.direction.y), 10, 50, 20, WHITE);

            // Draw UI
            // Weapons
            DrawText(TextFormat("Inventory:"), 10, 180, 20, WHITE);
            for (int w = 0; w < PLAYER_MAX_WEAPONS; w++) {
                DrawText(TextFormat("%d: %s",w, player.weapons[w].name.c_str()), 10, 200 + (w * 20), 20, WHITE);
            }
            // Keyes
            for (int k = 0; k < PLAYER_MAX_KEYS; k++) {
                DrawText(TextFormat("%d: %s",k + PLAYER_MAX_WEAPONS, player.keys[k].name.c_str()), 10, (200 + (PLAYER_MAX_WEAPONS * 20)) + (k * 20), 20, WHITE);
            }

            // Hp and Sp
            DrawText(TextFormat("Health: %d/%d", player.hp, PLAYER_MAX_HP), 200, window.height - 30, 20, WHITE);
            DrawText(TextFormat("Shield: %d/%d", player.sp, PLAYER_MAX_SP), 350, window.height - 30, 20, WHITE);

            ImageClearBackground(&screen_buffer_image, Color{0, 0, 0, 0});


            EndDrawing();
        }
    };
}
