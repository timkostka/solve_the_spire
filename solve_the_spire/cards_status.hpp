#pragma once

#include "cards.hpp"

const Card card_wound = {
    "Wound", 0, 0, nullptr,
    {.status = 1, .unplayable = 1},
    kActionNone};

const Card card_dazed = {
    "Dazed", 0, 0, nullptr,
    {.status = 1, .unplayable = 1, .ethereal = 1},
    kActionNone};

const Card card_burn = {
    "Burn", 0, 0, nullptr,
    {.status = 1, .unplayable = 1},
    kActionNone};
