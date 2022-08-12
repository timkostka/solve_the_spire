#pragma once

#include "cards.hpp"

// starting defect cards

const Card card_zap_plus = {
    "Zap+", 0, 0, nullptr,
    {.skill = 1, .starting = 1, .defect = 1},
    {{kActionChannelOrb, kOrbLightning, 1}, {kActionNone}}};
const Card card_zap = {
    "Zap", 1, 1, &card_zap_plus,
    {.skill = 1, .starting = 1, .defect = 1},
    {{kActionChannelOrb, kOrbLightning, 1}, {kActionNone}}};

const Card card_dualcast_plus = {
    "Dualcast+", 0, 0, nullptr,
    {.skill = 1, .starting = 1, .defect = 1},
    {{kActionEvokeOrb, 2}, {kActionNone}}};
const Card card_dualcast = {
    "Dualcast", 1, 1, &card_dualcast_plus,
    {.skill = 1, .starting = 1, .defect = 1},
    {{kActionEvokeOrb, 2}, {kActionNone}}};
