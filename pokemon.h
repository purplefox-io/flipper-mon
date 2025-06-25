// pokemon.h - Contains Pokemon data structures and move definitions
#ifndef POKEMON_H
#define POKEMON_H

// Define Pokemon species
typedef enum {
    POKEMON_BULBASAUR,
    POKEMON_CHARMANDER,
    POKEMON_SQUIRTLE,
    POKEMON_PIDGEY,
    POKEMON_ZUBAT,
    POKEMON_COUNT
} PokemonSpecies;

// Define move types
typedef enum {
    MOVE_TYPE_NORMAL,
    MOVE_TYPE_FIRE,
    MOVE_TYPE_WATER,
    MOVE_TYPE_GRASS,
    MOVE_TYPE_ELECTRIC,
    MOVE_TYPE_FLYING,
    MOVE_TYPE_POISON,
    MOVE_TYPE_COUNT
} MoveType;

// Define move effects
typedef enum {
    EFFECT_NONE,
    EFFECT_BURN,
    EFFECT_PARALYZE,
    EFFECT_SLEEP,
    EFFECT_POISON,
    EFFECT_COUNT
} MoveEffect;

// Move structure
typedef struct {
    const char* name;
    MoveType type;
    int power;
    int accuracy;
    MoveEffect effect;
    int effect_chance;
} Move;

// Define all possible moves
extern const Move all_moves[];

// Pokemon can have up to 4 moves
typedef struct {
    const char* name;
    PokemonSpecies species;
    int level;
    int max_hp;
    int current_hp;
    int attack;
    int defense;
    int speed;
    const unsigned char* front_sprite;
    const unsigned char* back_sprite;
    Move moves[4];
} Pokemon;


// Default move sets per Pokemon species
extern const Move* default_moves[POKEMON_COUNT][4];

// Initialize a new Pokemon
Pokemon create_pokemon(PokemonSpecies species, int level);

// Calculate damage for a move
int calculate_damage(Move move, Pokemon attacker, Pokemon defender);


extern const unsigned char bulbasaur[];




#endif // POKEMON_H