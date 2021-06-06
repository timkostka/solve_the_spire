#pragma once

#include <cstdint>

// max number of mobs per node
constexpr unsigned int MAX_MOBS_PER_NODE = 2;

// max number of actions per card
constexpr unsigned int MAX_CARD_ACTIONS = 5;

// max number of cards in hand
constexpr unsigned int MAX_HAND_SIZE = 10;

// if true, print out stats before/after each pruning
constexpr bool print_around_pruning = false;

// if true, use average HP values when generating mobs
constexpr bool normalize_mob_variations = true;

// if true, keep entire tree in memory
// (this can increase memory requirements)
constexpr bool keep_entire_tree_in_memory = true;

// if true, print completed tree to file
constexpr bool print_completed_tree_to_file = true;

// maximum nodes to print to file
// (if tree is more than this many nodes, it will not be output to a file)
constexpr uint32_t max_nodes_to_print = 500000;

// if true, makes probably-better choices when Miracle is in hand
// (e.g. only play Miracle if we use the energy)
// TODO
//constexpr bool simplify_miracle_decisions = true;

// display all player choice nodes
// (this is for debugging only, significantly increases output)
constexpr bool show_player_choices = false;

//#define USE_ORBS
