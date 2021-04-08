#pragma once

#include <cstdint>

// max number of mobs per node
constexpr uint16_t MAX_MOBS_PER_NODE = 2;

// if true, print out stats before/after each pruning
constexpr bool print_around_pruning = false;

// if true, use average HP values when generating mobs
constexpr bool normalize_mob_variation = true;
