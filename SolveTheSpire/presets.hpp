#pragma once

#include <map>

#include "defines.h"
#include "card_collection_map.hpp"

// character types
struct CharacterType {
    // character name
    std::string name;
    // deck
    std::string deck;
    // relics
    std::string relics;
    // max hp
    uint16_t max_hp;
};

// map between characters and setup information
std::map<std::string, CharacterType> character_map = {
    {"ironclad", {"Ironclad", "starting_ironclad_cursed", "burning_blood", 80}},
    {"silent", {"Silent", "starting_silent_cursed", "ring_of_the_snake", 70}},
    {"defect", {"Defect", "starting_defect_cursed", "cracked_core", 75}},
    {"watcher", {"Watcher", "starting_watcher_cursed", "pure_water", 72}},
};

// map between deck names and decks
std::map<std::string, CardCollectionPtr> deck_map;
