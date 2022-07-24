#pragma once

#include <cstdint>

// max number of mobs per node
constexpr unsigned int MAX_MOBS_PER_NODE = 2;

// max number of mobs per node
constexpr unsigned int MAX_PENDING_ACTIONS = 2;

// max number of actions per card
constexpr unsigned int MAX_CARD_ACTIONS = 4;

// max number of cards in hand
constexpr unsigned int MAX_HAND_SIZE = 10;

// if true, print out stats before/after each pruning
constexpr bool print_around_pruning = false;

// if true, use average HP values when generating mobs
constexpr bool normalize_mob_variations = true;

// if true, print completed tree to file
constexpr bool print_completed_tree_to_file = true;

// maximum nodes to print to file
// (if tree is more than this many nodes, it will not be output to a file)
constexpr unsigned int max_nodes_to_print = 800000;

// if true, makes probably-better choices when Miracle is in hand
// (e.g. only play Miracle if we use the energy)
// TODO
//constexpr bool simplify_miracle_decisions = true;

// display all player choice nodes
// (this is for debugging only, significantly increases output)
constexpr bool show_player_choices = false;

// verify node tree after every expansion
// (this is for debugging only, significantly slows down program)
constexpr bool verify_all_expansions = false;

//#define USE_ORBS

// if true, will treat an upgraded card as strictly better than a non-upgraded one
constexpr bool upgrades_strictly_better = true;

// if true, will make choices to stay alive even if death is inevitable and more
// damage to the mob could be done
constexpr bool always_avoid_dying = false;

// when we reach this many nodes stored, stop storing the entire tree
constexpr unsigned int max_nodes_to_store = 10000000;
