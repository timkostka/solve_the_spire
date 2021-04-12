#pragma once

#include <cstdint>

// max number of mobs per node
constexpr uint16_t MAX_MOBS_PER_NODE = 2;

// if true, print out stats before/after each pruning
constexpr bool print_around_pruning = false;

// if true, use average HP values when generating mobs
constexpr bool normalize_mob_variations = true;

// if true, evaluate a critical path before
// (this can increase memory requirements)
//constexpr bool evaluate_critical_path = false;

// if true, keep entire tree in memory
// (this can increase memory requirements)
constexpr bool keep_entire_tree_in_memory = true;

// if true, print completed tree to file
constexpr bool print_completed_tree_to_file = true;
