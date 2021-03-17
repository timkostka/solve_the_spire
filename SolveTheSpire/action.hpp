#pragma once

#include <cstdint>

// action type
enum ActionType : uint8_t {
    // no action
    kActionNone,
    // attack a single enemy
    // first argument is attack amount
    kActionAttack,
    // attack all enemies
    // first argument is attack amount
    kActionAttackAll,
    // apply block
    // first argument is block amount
    kActionBlock,
    // apply a buff to the player
    // first argument is buff type
    // second argument is buff amount
    kActionBuff,
    // apply a debuff/buff to a single enemy
    // first argument is buff type
    // second argument is buff amount
    kActionDebuff,
    // apply a debuff/buff to all enemies
    // first argument is buff type
    // second argument is buff amount
    kActionDebuffAll,
};

// An action is a single event caused by a card such as damage, or a buff
struct Action {
    // action type
    ActionType type;
    // arguments (meaning depends on action type)
    int16_t arg[2];
};
