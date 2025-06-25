#include "pokemon.h"
#include <stdlib.h>

// Define all available moves
const Move all_moves[] = {
    // Normal moves
    {"Tackle", MOVE_TYPE_NORMAL, 40, 100, EFFECT_NONE, 0},
    {"Scratch", MOVE_TYPE_NORMAL, 40, 100, EFFECT_NONE, 0},
    {"Quick Attack", MOVE_TYPE_NORMAL, 40, 100, EFFECT_NONE, 0},
    {"Growl", MOVE_TYPE_NORMAL, 0, 100, EFFECT_NONE, 0},
    
    // Fire moves
    {"Ember", MOVE_TYPE_FIRE, 40, 100, EFFECT_BURN, 10},
    {"Flamethrower", MOVE_TYPE_FIRE, 90, 100, EFFECT_BURN, 10},
    
    // Water moves
    {"Water Gun", MOVE_TYPE_WATER, 40, 100, EFFECT_NONE, 0},
    {"Bubble", MOVE_TYPE_WATER, 40, 100, EFFECT_NONE, 0},
    
    // Grass moves
    {"Vine Whip", MOVE_TYPE_GRASS, 45, 100, EFFECT_NONE, 0},
    {"Razor Leaf", MOVE_TYPE_GRASS, 55, 95, EFFECT_NONE, 0},
    
    // Flying moves
    {"Gust", MOVE_TYPE_FLYING, 40, 100, EFFECT_NONE, 0},
    {"Wing Attack", MOVE_TYPE_FLYING, 60, 100, EFFECT_NONE, 0},
    
    // Poison moves
    {"Poison Sting", MOVE_TYPE_POISON, 15, 100, EFFECT_POISON, 30},
    {"Acid", MOVE_TYPE_POISON, 40, 100, EFFECT_POISON, 10},
    
    // Electric moves
    {"Thunder Shock", MOVE_TYPE_ELECTRIC, 40, 100, EFFECT_PARALYZE, 10},
};

// Default move sets for each Pokémon species
const Move* default_moves[POKEMON_COUNT][4] = {
    // BULBASAUR
    {&all_moves[0], &all_moves[3], &all_moves[8], &all_moves[12]}, // Tackle, Growl, Vine Whip, Poison Sting
    
    // CHARMANDER
    {&all_moves[1], &all_moves[3], &all_moves[4], NULL}, // Scratch, Growl, Ember
    
    // SQUIRTLE
    {&all_moves[0], &all_moves[3], &all_moves[6], NULL}, // Tackle, Growl, Water Gun
    
    // PIDGEY
    {&all_moves[0], &all_moves[2], &all_moves[10], NULL}, // Tackle, Quick Attack, Gust
    
    // ZUBAT
    {&all_moves[12], &all_moves[10], &all_moves[13], NULL}  // Poison Sting, Gust, Acid
};

// Base stats for each Pokémon species (HP, Attack, Defense, Speed)
static const int base_stats[POKEMON_COUNT][4] = {
    {45, 49, 49, 45},   // BULBASAUR
    {39, 52, 43, 65},   // CHARMANDER
    {44, 48, 65, 43},   // SQUIRTLE
    {40, 45, 40, 56},   // PIDGEY
    {40, 45, 35, 55},   // ZUBAT
};

// Get sprite for a Pokémon species
static const unsigned char* get_pokemon_sprite(PokemonSpecies species) {
    switch (species) {
        case POKEMON_BULBASAUR:
            return bulbasaur;
        // Add other Pokémon sprites as they become available
        default:
            return bulbasaur; // Default to bulbasaur if sprite not available
    }
}

// Create a new Pokémon with given species and level
Pokemon create_pokemon(PokemonSpecies species, int level) {
    Pokemon pokemon;
    
    // Set basic info
    pokemon.species = species;
    pokemon.level = level;
    
    // Set name based on species
    switch (species) {
        case POKEMON_BULBASAUR:
            pokemon.name = "Bulbasaur";
            break;
        case POKEMON_CHARMANDER:
            pokemon.name = "Charmander";
            break;
        case POKEMON_SQUIRTLE:
            pokemon.name = "Squirtle";
            break;
        case POKEMON_PIDGEY:
            pokemon.name = "Pidgey";
            break;
        case POKEMON_ZUBAT:
            pokemon.name = "Zubat";
            break;
        default:
            pokemon.name = "???";
            break;
    }
    
    // Calculate stats based on level and base stats
    pokemon.max_hp = (base_stats[species][0] * 2 * level) / 100 + level + 10;
    pokemon.current_hp = pokemon.max_hp;
    pokemon.attack = (base_stats[species][1] * 2 * level) / 100 + 5;
    pokemon.defense = (base_stats[species][2] * 2 * level) / 100 + 5;
    pokemon.speed = (base_stats[species][3] * 2 * level) / 100 + 5;
    
    // Set sprite
    pokemon.front_sprite = get_pokemon_sprite(species);
    pokemon.back_sprite = get_pokemon_sprite(species); // Use same sprite for now
    
    // Set moves based on default move set
    for (int i = 0; i < 4; i++) {
        if (default_moves[species][i] != NULL) {
            pokemon.moves[i] = *default_moves[species][i];
        } else {
            // Empty move slot
            pokemon.moves[i].name = "";
            pokemon.moves[i].power = 0;
        }
    }
    
    return pokemon;
}

// Calculate damage for a move
int calculate_damage(Move move, Pokemon attacker, Pokemon defender) {
    if (move.power == 0) return 0; // Status moves deal no damage
    
    // Simple damage formula: (2 * Level * Power * (Attack / Defense)) / 50 + 2
    int damage = (2 * attacker.level * move.power * attacker.attack) / (defender.defense * 50) + 2;
    
    // Apply random factor (85-100%)
    damage = (damage * (85 + (rand() % 16))) / 100;
    
    // Apply STAB (Same Type Attack Bonus)
    // Would need to add Pokemon types to fully implement
    
    return damage > 0 ? damage : 1; // Minimum damage is 1
}




// Pokemon sprites
const unsigned char bulbasaur [] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x00, 
	0xc0, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 
	0x30, 0x00, 0x30, 0xc0, 0x00, 0x00, 0xc8, 0xfe, 0x4f, 0x40, 0x01, 0x00, 0x88, 0x01, 0x42, 0x40, 
	0x02, 0x00, 0x28, 0x38, 0x40, 0x80, 0x02, 0x00, 0x18, 0x3c, 0xc0, 0x80, 0x04, 0x00, 0x08, 0x1e, 
	0xc0, 0x00, 0x05, 0x00, 0x08, 0x0e, 0xc6, 0x00, 0x09, 0x00, 0x0c, 0x00, 0xe9, 0x01, 0x09, 0x00, 
	0x1c, 0x8c, 0xf3, 0x03, 0x09, 0x00, 0x1c, 0x8e, 0xf3, 0x83, 0x08, 0x00, 0x2a, 0xc6, 0xe6, 0x87, 
	0x08, 0x00, 0x2a, 0xc0, 0xe0, 0x4f, 0x04, 0x00, 0x3a, 0xc0, 0xf1, 0x3f, 0x04, 0x00, 0x06, 0x00, 
	0xf8, 0x7f, 0x02, 0x00, 0x9e, 0x04, 0xfe, 0xff, 0x01, 0x00, 0x6c, 0xf0, 0xfd, 0xff, 0x01, 0x00, 
	0xf8, 0xff, 0xff, 0xff, 0x03, 0x00, 0xf0, 0x87, 0xff, 0xff, 0x03, 0x00, 0xc0, 0xff, 0xff, 0xff, 
	0x07, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x07, 0x00, 0x40, 0xfc, 0xe7, 0xff, 0x07, 0x00, 0x80, 0xf9, 
	0xcf, 0xff, 0x07, 0x00, 0x80, 0xf9, 0xdf, 0xff, 0x03, 0x00, 0x80, 0x70, 0xda, 0xfd, 0x03, 0x00, 
	0x00, 0x75, 0xc2, 0x54, 0x01, 0x00, 0x80, 0x3a, 0xea, 0xf8, 0x00, 0x00, 0x00, 0x1f, 0x55, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};