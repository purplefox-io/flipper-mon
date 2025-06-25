#include "furi.h"
#include "gui/gui.h"
#include "input/input.h"
#include "flipper_mon_icons.h"
#include <gui/icon_i.h>
#include <stdlib.h> // Required for rand()
#include "tiles.h"
#include "sprites.h"
#include "maps.h"
#include "pokemon.h"

#define TILE_SIZE     16          // New tile size: 16x16 pixels
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define HP_BAR_WIDTH  40
#define HP_BAR_HEIGHT 6
#define BATTLE_BORDER_OFFSET 5


// Struct for a Map
typedef struct {
    const char* name;
    int width;
    int height;
    Tile tiles[MAP_WIDTH][MAP_HEIGHT];
} GameMap;

// Array of all maps
static GameMap maps[MAX_MAPS];
// Index of current active map
static int current_map_index = 0;
#define CURRENT_MAP (&maps[current_map_index]) // Macro for current map

// Scene definitions
typedef enum {
    SceneExploration,
    SceneBattle,
    SceneWildBattle,
    SceneCutscene,
} GameScene;

// Scene Manager
typedef struct {
    GameScene current_scene;
} SceneManager;

static SceneManager scene_manager = {.current_scene = SceneExploration};

// Define the map dimensions (in tiles).
#define MAP_WIDTH  20
#define MAP_HEIGHT 20

// Custom event types.
typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

// PluginEvent wraps an input event.
typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

static int anim_frame = 0;

// Player's Pokemon - using the new Pokemon struct from pokemon.h
static Pokemon player_pokemon;

// Wild Pokemon for battles
static Pokemon wild_pokemon;

// Global trainer structure.
typedef struct {
    int x;
    int y;
    int direction; // 0: up, 1: right, 2: down, 3: left
} Trainer;

static Trainer trainer = { .x = 32, .y = 32, .direction = 2 };



// Battle UI states
typedef enum {
    BattleStateIntro,      // "A wild X appeared!"
    BattleStateChooseAction, // "Fight/Item/Run"
    BattleStateChooseMove,  // Select from available moves
    BattleStateExecuteMove, // Show move animation and effects
    BattleStateEnemyTurn,   // Enemy attacks
    BattleStateResult,      // Show results (hit/miss/effective)
    BattleStateEnd          // Battle ending (victory/defeat/ran)
} BattleState;

// Dialog box structure
typedef struct {
    char text[64];
    bool is_active;
    int cursor_position;
    int option_count;
} DialogBox;

// Add these to your existing global variables
static BattleState battle_state = BattleStateIntro;
static DialogBox dialog_box = {.is_active = false, .cursor_position = 0, .option_count = 0};
static int battle_animation_frame = 0;
static int battle_animation_timer = 0;
static bool player_turn = true;
static int selected_move_index = 0;
static char battle_result_text[64] = "";
static int damage_dealt = 0;

// Helper function to draw health bar
static void draw_health_bar_opponent(Canvas* canvas, int x, int y, int width, int height, int current_hp, int max_hp) {
    int filled_width = (current_hp * width) / max_hp;

    // Draw border
    canvas_draw_frame(canvas, x, y, width, height);

    // Fill the health bar
    canvas_draw_box(canvas, x + 1, y + 1, filled_width - 2, height - 2);

    // Draw HP text next to bar
    char hp_text[8];
    snprintf(hp_text, sizeof(hp_text), "%d HP", current_hp);
    canvas_draw_str(canvas, x + width - width, y + height + 8, hp_text);
}

// Helper function to draw health bar
static void draw_health_bar_player(Canvas* canvas, int x, int y, int width, int height, int current_hp, int max_hp) {
    int filled_width = (current_hp * width) / max_hp;
    // Draw HP text next to bar
    char hp_text[8];
    snprintf(hp_text, sizeof(hp_text), "%d HP", current_hp);
    canvas_draw_str(canvas, x, y + height +10, hp_text);
    canvas_draw_frame(canvas, x, y + 2, width, height);

    // Fill the health bar
    canvas_draw_box(canvas, x + 1, y + 3, filled_width - 2, height - 2);
}


// Helper function to draw a dialog box
static void draw_dialog_box(Canvas* canvas, int x, int y, int width, int height) {
    // Draw box background
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x, y, width, height);
    
    // Draw border
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, x, y, width, height);
}

// Draw the battle menu options
static void draw_battle_menu(Canvas* canvas, int x, int y) {
    draw_dialog_box(canvas, x, y, 80, 24);
    
    const char* options[4] = {"FIGHT", "PKMN", "ITEM", "RUN"};
    
    for(int i = 0; i < 4; i++) {
        int option_x = x + 5 + (i % 2) * 40;
        int option_y = y + 8 + (i / 2) * 12;
        
        // Highlight selected option
        if(dialog_box.cursor_position == i) {
            canvas_draw_str(canvas, option_x - 2, option_y, ">");
        }
        
        canvas_draw_str(canvas, option_x + 5, option_y, options[i]);
    }
}

// Draw the move selection menu
static void draw_move_menu(Canvas* canvas, int x, int y) {
    draw_dialog_box(canvas, x, y, 120, 24);
    
    for(int i = 0; i < 4; i++) {
        int option_x = x + 5 + (i % 2) * 60;
        int option_y = y + 8 + (i / 2) * 12;
        
        // Skip empty move slots
        if(player_pokemon.moves[i].name[0] == '\0') continue;
        
        // Highlight selected move
        if(dialog_box.cursor_position == i) {
            canvas_draw_str(canvas, option_x - 2, option_y, ">");
        }
        
        canvas_draw_str(canvas, option_x + 5, option_y, player_pokemon.moves[i].name);
    }
}

// Corrected draw_dialog_text function using manual string parsing
static void draw_dialog_text(Canvas* canvas, int x, int y, int width, int height) {
    draw_dialog_box(canvas, x, y, width, height);

    char text_copy[64];
    strncpy(text_copy, dialog_box.text, sizeof(text_copy));
    text_copy[sizeof(text_copy) - 1] = '\0';

    int line_y = y + 10;
    char* start = text_copy;
    char* end;

    while((end = strchr(start, '\n')) != NULL) {
        // Temporarily terminate the string at the newline
        *end = '\0';
        canvas_draw_str(canvas, x + 5, line_y, start);
        line_y += 10;
        // Move start to the character after the newline
        start = end + 1;
    }

    // Print the last remaining part of the string (or the only part if no newlines)
    if (*start != '\0') {
        canvas_draw_str(canvas, x + 5, line_y, start);
    }
}

// Update the battle UI based on current state
static void update_battle_ui(void) {
    switch(battle_state) {
        case BattleStateIntro:
            snprintf(dialog_box.text, sizeof(dialog_box.text), "A wild %s appeared!", wild_pokemon.name);
            dialog_box.is_active = true;
            dialog_box.option_count = 0;
            break;
            
        case BattleStateChooseAction:
            dialog_box.is_active = true;
            dialog_box.option_count = 4;
            dialog_box.cursor_position = 0;
            break;
            
        case BattleStateChooseMove:
            dialog_box.is_active = true;
            // Count how many non-empty moves the player has
            dialog_box.option_count = 0;
            for(int i = 0; i < 4; i++) {
                if(player_pokemon.moves[i].name[0] != '\0') {
                    dialog_box.option_count++;
                }
            }
            dialog_box.cursor_position = 0;
            break;
            
        case BattleStateExecuteMove:
            snprintf(dialog_box.text, sizeof(dialog_box.text), "%s used %s!", 
                     player_pokemon.name, player_pokemon.moves[selected_move_index].name);
            dialog_box.is_active = true;
            dialog_box.option_count = 0;
            battle_animation_frame = 0;
            battle_animation_timer = 0;
            break;
            
        case BattleStateResult:
            snprintf(dialog_box.text, sizeof(dialog_box.text), "%s", battle_result_text);
            dialog_box.is_active = true;
            dialog_box.option_count = 0;
            break;
            
        case BattleStateEnemyTurn:
            // Randomly select a move for the wild Pokemon
            int valid_moves = 0;
            for(int i = 0; i < 4; i++) {
                if(wild_pokemon.moves[i].name[0] != '\0') {
                    valid_moves++;
                }
            }
            
            if(valid_moves == 0) {
                // If no valid moves (shouldn't happen), go back to player turn
                battle_state = BattleStateChooseAction;
                player_turn = true;
                break;
            }
            
            // Select a random valid move
            int enemy_move_index = rand() % valid_moves;
            int actual_index = 0;
            for(int i = 0; i < 4; i++) {
                if(wild_pokemon.moves[i].name[0] != '\0') {
                    if(actual_index == enemy_move_index) {
                        enemy_move_index = i;
                        break;
                    }
                    actual_index++;
                }
            }
            
            // Execute the enemy move
            snprintf(dialog_box.text, sizeof(dialog_box.text), "Wild %s used %s!", 
                     wild_pokemon.name, wild_pokemon.moves[enemy_move_index].name);
            dialog_box.is_active = true;
            dialog_box.option_count = 0;
            
            // Calculate damage to player
            damage_dealt = calculate_damage(wild_pokemon.moves[enemy_move_index], wild_pokemon, player_pokemon);
            player_pokemon.current_hp -= damage_dealt;
            if(player_pokemon.current_hp < 0) player_pokemon.current_hp = 0;
            
            battle_animation_frame = 0;
            battle_animation_timer = 0;
            break;
            
        case BattleStateEnd:
            if(wild_pokemon.current_hp <= 0) {
                snprintf(dialog_box.text, sizeof(dialog_box.text), "Wild %s fainted!", wild_pokemon.name);
            } else if(player_pokemon.current_hp <= 0) {
                snprintf(dialog_box.text, sizeof(dialog_box.text), "%s fainted!", player_pokemon.name);
            }
            dialog_box.is_active = true;
            dialog_box.option_count = 0;
            break;
    }
}

// Execute a player's move
void execute_player_move(int move_index) {
    selected_move_index = move_index;
    
    // Calculate damage
    Move selected_move = player_pokemon.moves[move_index];
    damage_dealt = calculate_damage(selected_move, player_pokemon, wild_pokemon);
    
    // Apply damage to wild Pokemon
    wild_pokemon.current_hp -= damage_dealt;
    if(wild_pokemon.current_hp < 0) wild_pokemon.current_hp = 0;
    
    // Set result text
    if(damage_dealt > 0) {
        snprintf(battle_result_text, sizeof(battle_result_text), "It did %d damage!", damage_dealt);
    } else {
        snprintf(battle_result_text, sizeof(battle_result_text), "It had no effect...");
    }
    
    battle_state = BattleStateExecuteMove;
    update_battle_ui();
}

// Process battle input
static void process_battle_input(InputKey key) {
    switch(battle_state) {
        case BattleStateIntro:
            if(key == InputKeyOk) {
                battle_state = BattleStateChooseAction;
                update_battle_ui();
            }
            break;
            
        case BattleStateChooseAction:
            switch(key) {
                case InputKeyUp:
                    dialog_box.cursor_position = (dialog_box.cursor_position - 2) % 4;
                    if(dialog_box.cursor_position < 0) dialog_box.cursor_position += 4;
                    break;
                case InputKeyDown:
                    dialog_box.cursor_position = (dialog_box.cursor_position + 2) % 4;
                    break;
                case InputKeyLeft:
                    dialog_box.cursor_position = (dialog_box.cursor_position - 1) % 4;
                    if(dialog_box.cursor_position < 0) dialog_box.cursor_position += 4;
                    break;
                case InputKeyRight:
                    dialog_box.cursor_position = (dialog_box.cursor_position + 1) % 4;
                    break;
                case InputKeyOk:
                    if(dialog_box.cursor_position == 0) {
                        // Fight option selected
                        battle_state = BattleStateChooseMove;
                        update_battle_ui();
                    } else if(dialog_box.cursor_position == 3) {
                        // Run option selected
                        scene_manager.current_scene = SceneExploration;
                    }
                    break;
                case InputKeyBack:
                    scene_manager.current_scene = SceneExploration;
                    break;
                default:
                    break;
            }
            break;
            
        case BattleStateChooseMove:
            switch(key) {
                case InputKeyUp:
                    dialog_box.cursor_position = (dialog_box.cursor_position - 2) % dialog_box.option_count;
                    if(dialog_box.cursor_position < 0) dialog_box.cursor_position += dialog_box.option_count;
                    break;
                case InputKeyDown:
                    dialog_box.cursor_position = (dialog_box.cursor_position + 2) % dialog_box.option_count;
                    break;
                case InputKeyLeft:
                    dialog_box.cursor_position = (dialog_box.cursor_position - 1) % dialog_box.option_count;
                    if(dialog_box.cursor_position < 0) dialog_box.cursor_position += dialog_box.option_count;
                    break;
                case InputKeyRight:
                    dialog_box.cursor_position = (dialog_box.cursor_position + 1) % dialog_box.option_count;
                    break;
                case InputKeyOk:
                    execute_player_move(dialog_box.cursor_position);
                    break;
                case InputKeyBack:
                    battle_state = BattleStateChooseAction;
                    update_battle_ui();
                    break;
                default:
                    break;
            }
            break;
            
        case BattleStateExecuteMove:
            if(key == InputKeyOk && battle_animation_timer > 20) {
                battle_state = BattleStateResult;
                update_battle_ui();
            }
            break;
            
        case BattleStateResult:
            if(key == InputKeyOk) {
                // First, check if the battle has ended
                if(wild_pokemon.current_hp <= 0 || player_pokemon.current_hp <= 0) {
                    battle_state = BattleStateEnd;
                    update_battle_ui();
                } else {
                    // If the battle is not over, check whose turn just ended
                    if(player_turn) {
                        // It was the player's turn, so now it's the enemy's turn
                        battle_state = BattleStateEnemyTurn;
                        player_turn = false;
                        update_battle_ui();
                    } else {
                        // It was the enemy's turn, so now it's the player's turn again
                        battle_state = BattleStateChooseAction;
                        player_turn = true;
                        update_battle_ui();
                    }
                }
            }
            break;
            
        case BattleStateEnemyTurn:
            if(key == InputKeyOk && battle_animation_timer > 20) {
                snprintf(battle_result_text, sizeof(battle_result_text), "It did %d damage!", damage_dealt);
                battle_state = BattleStateResult;
                update_battle_ui();
            }
            break;
            
        case BattleStateEnd:
            if(key == InputKeyOk) {
                scene_manager.current_scene = SceneExploration;
            }
            break;
    }
}

// Update the battle animation frame
static void update_battle_animation(void) {
    battle_animation_timer++;
    
    if(battle_animation_timer % 5 == 0) {
        battle_animation_frame++;
    }
}

// Modify the handle_battle_input function to use our new process_battle_input
void handle_battle_input(PluginEvent* event) {
    if(event->input.type == InputTypePress) {
        process_battle_input(event->input.key);
    }
}

// Update the draw_battle_scene function to include our new UI elements
static void draw_battle_scene(Canvas* canvas) {
    canvas_clear(canvas);

    // Draw background
    canvas_set_color(canvas, ColorBlack);
    
    // Draw opponent Pokemon
    int opponent_x = SCREEN_WIDTH - 60;
    int opponent_y = 0;
    canvas_draw_xbm(canvas, opponent_x, opponent_y, 42, 42, wild_pokemon.front_sprite);
    
    // Draw opponent info
    int opp_hp_x = 5, opp_hp_y = 5;
    char opp_level_text[16];
    snprintf(opp_level_text, sizeof(opp_level_text), "Wild %s LV%d", wild_pokemon.name, wild_pokemon.level);
    canvas_draw_str(canvas, opp_hp_x, opp_hp_y, opp_level_text);
    draw_health_bar_opponent(canvas, opp_hp_x, opp_hp_y + 5, HP_BAR_WIDTH, HP_BAR_HEIGHT, 
                          wild_pokemon.current_hp, wild_pokemon.max_hp);
    
    // Draw player Pokemon
    int player_x = 20;
    int player_y = SCREEN_HEIGHT - 40;
    canvas_draw_xbm(canvas, player_x, player_y, 42, 42, player_pokemon.back_sprite);
    
    // Draw player info
    int player_hp_x = SCREEN_WIDTH - 70;
    int player_hp_y = SCREEN_HEIGHT - 25;
    char player_level_text[16];
    snprintf(player_level_text, sizeof(player_level_text), "%s LV%d", player_pokemon.name, player_pokemon.level);
    canvas_draw_str(canvas, player_hp_x, player_hp_y, player_level_text);
    draw_health_bar_player(canvas, player_hp_x, player_hp_y + 5, HP_BAR_WIDTH, HP_BAR_HEIGHT, 
                        player_pokemon.current_hp, player_pokemon.max_hp);
    
    // Draw UI based on battle state
    switch(battle_state) {
        case BattleStateIntro:
        case BattleStateExecuteMove:
        case BattleStateResult:
        case BattleStateEnemyTurn:
        case BattleStateEnd:
            draw_dialog_text(canvas, 2, SCREEN_HEIGHT - 20, SCREEN_WIDTH - 4, 18);
            break;
            
        case BattleStateChooseAction:
            draw_battle_menu(canvas, SCREEN_WIDTH - 82, SCREEN_HEIGHT - 26);
            break;
            
        case BattleStateChooseMove:
            draw_move_menu(canvas, 4, SCREEN_HEIGHT - 26);
            break;
    }
    
    // Animation effects for attacks
    if(battle_state == BattleStateExecuteMove && battle_animation_frame < 3) {
        // Simple shake animation for player attack
        int shake_offset = (battle_animation_frame % 2 == 0) ? 2 : -2;
        canvas_draw_line(canvas, opponent_x + shake_offset, 0, opponent_x + 42 + shake_offset, 42);
    }
    
    if(battle_state == BattleStateEnemyTurn && battle_animation_frame < 3) {
        // Simple shake animation for enemy attack
        int shake_offset = (battle_animation_frame % 2 == 0) ? 2 : -2;
        canvas_draw_line(canvas, player_x + shake_offset, player_y, player_x + 42 + shake_offset, player_y + 42);
    }
}


// Clamp helper.
static inline int clamp(int value, int min, int max) {
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

// Get full map dimensions in pixels.
static inline int full_map_width_pixels(void) {
    return MAP_WIDTH * TILE_SIZE;
}

static inline int full_map_height_pixels(void) {
    return MAP_HEIGHT * TILE_SIZE;
}

// ---------------- SCENES ---------------- //

// **Exploration Scene**
static void draw_exploration_scene(Canvas* canvas) {
    canvas_clear(canvas);
    int camera_x = trainer.x - SCREEN_WIDTH / 2;
    int camera_y = trainer.y - SCREEN_HEIGHT / 2;
    camera_x = clamp(camera_x, 0, full_map_width_pixels() - SCREEN_WIDTH);
    camera_y = clamp(camera_y, 0, full_map_height_pixels() - SCREEN_HEIGHT);

    int start_tile_x = camera_x / TILE_SIZE;
    int start_tile_y = camera_y / TILE_SIZE;
    int end_tile_x = (camera_x + SCREEN_WIDTH - 1) / TILE_SIZE;
    int end_tile_y = (camera_y + SCREEN_HEIGHT - 1) / TILE_SIZE;

    for(int ty = start_tile_y; ty <= end_tile_y; ty++) {
        for(int tx = start_tile_x; tx <= end_tile_x; tx++) {
            Tile tile = CURRENT_MAP->tiles[tx][ty];
            int px = tx * TILE_SIZE - camera_x;
            int py = ty * TILE_SIZE - camera_y;

            // Grass
            if (tile.is_obstacle && (tile.y == 0 || tile.y ==  20))
                canvas_draw_xbm(canvas, px, py, TILE_SIZE, TILE_SIZE, fence_top_bottom);
            else
                canvas_draw_xbm(canvas, px, py, TILE_SIZE, TILE_SIZE, grass);

            if (tile.transition_map_index != -1) {
                canvas_draw_box(canvas, px, py, TILE_SIZE, TILE_SIZE);
            }
        }
    }

    int trainer_x = clamp(trainer.x, 0, full_map_width_pixels() - TILE_SIZE);
    int trainer_y = clamp(trainer.y, 0, full_map_height_pixels() - TILE_SIZE);
    int draw_x = trainer_x - camera_x;
    int draw_y = trainer_y - camera_y;

    const unsigned char* sprite = trainer_forward_normal;
    switch(trainer.direction) {
        case 1: sprite = (anim_frame % 3 == 0) ? trainer_backward_standing : (anim_frame % 3 == 1) ? trainer_backwards_walking_left : trainer_backwards_walking_right; break;
        case 2: sprite = (anim_frame % 2 == 0) ? trainer_right_standing : trainer_right_walking; break;
        case 3: sprite = (anim_frame % 3 == 0) ? trainer_forward_normal : (anim_frame % 3 == 1) ? trainer_front_walking_left : trainer_front_walking_right; break;
        case 4: sprite = (anim_frame % 2 == 0) ? trainer_left_standing : trainer_left_walking; break;
    }
    canvas_draw_xbm(canvas, draw_x, draw_y, TILE_SIZE, TILE_SIZE, sprite);
}

// Start a battle with a wild Pokemon
void start_battle(PokemonSpecies species, int level) {
    // Create a new wild Pokemon of the specified species and level
    wild_pokemon = create_pokemon(species, level);
    
    FURI_LOG_D("Game", "Wild %s (Lv %d) appeared!", wild_pokemon.name, wild_pokemon.level);
    
    // Reset battle state
    battle_state = BattleStateIntro;
    player_turn = true;
    dialog_box.cursor_position = 0;
    
    // Switch to battle scene
    scene_manager.current_scene = SceneBattle;
    
    // Initialize battle UI
    update_battle_ui();
}

bool check_for_encounter(int x, int y) {
    Tile* current_tile = &CURRENT_MAP->tiles[x / TILE_SIZE][y / TILE_SIZE];

    if (current_tile->spawn_data.spawn_rate > 0) {
        int roll = rand() % 100; // Roll from 0-99
        if (roll < current_tile->spawn_data.spawn_rate) {
            // Randomly pick a Pokémon from the tile's possible spawns
            int spawn_index = rand() % 3;
            PokemonSpecies wild_species = current_tile->spawn_data.pokemon[spawn_index];
            
            // Random level between min and max for the area
            int wild_level = current_tile->spawn_data.min_level + 
                (rand() % (current_tile->spawn_data.max_level - current_tile->spawn_data.min_level + 1));
                
            // Start battle with the wild Pokemon
            start_battle(wild_species, wild_level);
            return true;
        }
    }
    return false;
}


// The draw callback draws the visible portion of the map and then draws the trainer.
static void game_draw_callback(Canvas* canvas, void* ctx) {
    (void)ctx;
    canvas_clear(canvas);
    if (scene_manager.current_scene == SceneExploration) {
        draw_exploration_scene(canvas);
    } else {
        draw_battle_scene(canvas);
    }
}

void initialize_maps() {
    // **Route 1 Initialization**
    maps[0] = (GameMap){ .name = "Route 1", .width = MAP_WIDTH, .height = MAP_HEIGHT };

    for(int y = 0; y < MAP_HEIGHT; y++) {
        for(int x = 0; x < MAP_WIDTH; x++) {
            bool is_border = (y == 0 || y == MAP_HEIGHT - 1 || x == 0 || x == MAP_WIDTH - 1);
            maps[0].tiles[x][y] = (Tile){
                .x = x,
                .y = y,
                .type = TILE_TYPE_GRASS,
                .is_obstacle = is_border, // Border is obstacle
                .transition_map_index = -1,
                .spawn_data = GRASS_PEWTER, // 20% chance of Pokémon spawning
            };
        }
    }
    maps[0].tiles[4][MAP_HEIGHT - 2].transition_map_index = 1;

    // **Pallet Town Initialization**
    maps[1] = (GameMap){ .name = "Pallet Town", .width = MAP_WIDTH, .height = MAP_HEIGHT };

    for(int y = 0; y < MAP_HEIGHT; y++) {
        for(int x = 0; x < MAP_WIDTH; x++) {
            bool is_border = (y == 0 || y == MAP_HEIGHT - 1 || x == 0 || x == MAP_WIDTH - 1);
            maps[1].tiles[x][y] = (Tile){
                .x = x,
                .y = y,
                .type = TILE_TYPE_GRASS,
                .is_obstacle = is_border,
                .transition_map_index = -1,
                .spawn_data = GRASS_PEWTER, // 10% chance of Pokémon spawning
            };
        }
    }
}

bool check_map_transition(int x, int y) {
    int tile_x = x / TILE_SIZE;
    int tile_y = y / TILE_SIZE;

    // **Ensure tile indices are within valid map range**
    if (tile_x < 0 || tile_x >= MAP_WIDTH || tile_y < 0 || tile_y >= MAP_HEIGHT) {
        return false;
    }

    Tile* current_tile = &CURRENT_MAP->tiles[tile_x][tile_y];

    // **Check if the tile is a transition tile**
    if (current_tile->transition_map_index != -1) {
        int new_map_index = current_tile->transition_map_index;

        FURI_LOG_D("Game", "Transitioning from %s to %s", CURRENT_MAP->name, maps[new_map_index].name);

        // **Update the current map index**
        current_map_index = new_map_index;

        // **Find a valid spawn location in the new map**
        // Here we place the player at a predefined safe spot (can be adjusted per map)
        trainer.x = TILE_SIZE * 2;  // Example spawn X position
        trainer.y = TILE_SIZE * (MAP_HEIGHT / 2);  // Example spawn Y position

        return true;
    }

    return false;
}


void handle_movement(PluginEvent* event) {
    // If in battle mode, handle battle input instead
    if (scene_manager.current_scene == SceneBattle) {
        handle_battle_input(event);
        return;
    }
    
    int new_x = trainer.x;
    int new_y = trainer.y;
    
    switch(event->input.key) {
        case InputKeyUp:
            new_y -= TILE_SIZE;
            trainer.direction = 1;
            break;
        case InputKeyRight:
            new_x += TILE_SIZE;
            trainer.direction = 2;
            break;
        case InputKeyDown:
            new_y += TILE_SIZE;
            trainer.direction = 3;
            break;
        case InputKeyLeft:
            new_x -= TILE_SIZE;
            trainer.direction = 4;
            break;
        case InputKeyOk:
            break;
        case InputKeyBack:
            break;
        default:
            break;
    }

    new_x = clamp(new_x, 0, CURRENT_MAP->width * TILE_SIZE - TILE_SIZE);
    new_y = clamp(new_y, 0, CURRENT_MAP->height * TILE_SIZE - TILE_SIZE);

    // **Ensure tile indices are within bounds**
    int tile_x = new_x / TILE_SIZE;
    int tile_y = new_y / TILE_SIZE;

    if(tile_x < 0 || tile_x >= MAP_WIDTH || tile_y < 0 || tile_y >= MAP_HEIGHT) {
        FURI_LOG_D("Game", "Invalid tile access at (%d, %d)", tile_x, tile_y);
        return;
    }

    Tile* next_tile = &CURRENT_MAP->tiles[tile_x][tile_y];

    if (next_tile->is_obstacle) {
        FURI_LOG_D("Game", "Blocked by an obstacle!");
        return;
    }

    if (check_map_transition(new_x, new_y)) return;

    if (!check_for_encounter(new_x, new_y)) {
        new_x = clamp(new_x, 0, full_map_width_pixels() - TILE_SIZE);
        new_y = clamp(new_y, 0, full_map_height_pixels() - TILE_SIZE);
        trainer.x = new_x;
        trainer.y = new_y;
        anim_frame++;
    }
}

// Add this to the existing while loop in app_main
void update_game_state(void) {
    // Update animations
    if(scene_manager.current_scene == SceneBattle) {
        update_battle_animation();
    }
}

// Input callback: post key events to the message queue.
static void input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = (FuriMessageQueue*)ctx;
    PluginEvent event = { .type = EventTypeKey, .input = *input_event };
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

int32_t app_main(void* p) {
    (void)p;
    // Allocate a message queue for PluginEvents.
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));
    if(!event_queue) {
        FURI_LOG_E("App", "Failed to allocate event queue");
        return 1;
    }

    // Initialize player's Pokemon - starting with Bulbasaur level 5
    player_pokemon = create_pokemon(POKEMON_BULBASAUR, 5);
    
    // **Initialize maps before anything else**
    initialize_maps();

    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, game_draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, event_queue);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);


    bool running = true;
    PluginEvent event;
    while(running) {
        // Wait for an event (100ms timeout).
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == EventTypeKey && event.input.type == InputTypePress) {
                handle_movement(&event);
            }
        }

         // Update game state - add this
        update_game_state();
        
        view_port_update(view_port);
    }

    // Clean up: disable callbacks before freeing resources.
    view_port_input_callback_set(view_port, NULL, NULL);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_record_close(RECORD_GUI);
    return 0;
}