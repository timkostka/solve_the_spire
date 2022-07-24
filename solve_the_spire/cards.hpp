#pragma once

#include <cstdint>
#include <cassert>
#include <vector>
#include <string>

#include "defines.h"
#include "orbs.hpp"
#include "action.hpp"
#include "buff_state.hpp"

struct Card;

// list of all cards in use
// map between card index and card
std::vector<const Card *> card_map;

// list of card flags
struct CardFlagStruct {
    unsigned int attack : 1;
    unsigned int skill : 1;
    unsigned int power : 1;
    unsigned int curse : 1;
    unsigned int status : 1;

    unsigned int unplayable : 1;

    // true if this targets an enemy
    unsigned int targeted : 1;

    // true if this targets a card in hand
    unsigned int target_card_in_hand : 1;

    // cost is X
    unsigned int x_cost : 1;

    unsigned int ethereal : 1;
    unsigned int exhausts : 1;
    // true if card name has "Strike"
    unsigned int strike : 1;
    unsigned int retain : 1;
    // TODO: implement this
    unsigned int innate : 1;

    unsigned int starting : 1;
    unsigned int common : 1;
    unsigned int uncommon : 1;
    unsigned int rare : 1;
    unsigned int special : 1;

    unsigned int upgraded : 1;

    unsigned int ironclad : 1;
    unsigned int silent : 1;
    unsigned int defect : 1;
    unsigned int watcher : 1;
    unsigned int colorless : 1;

};

static_assert(sizeof(CardFlagStruct) == 4, "");

// A Card holds information about a specific card.
struct Card {
    // text name as it appears
    std::string name;
    //CardType type;
    // base energy cost
    uint8_t base_cost;
    // current energy cost
    uint8_t cost;
    // upgraded card version, or nullptr
    const Card * upgraded_version;
    // card flags
    CardFlagStruct flag;
    // list of actions
    Action action[MAX_CARD_ACTIONS];
    // card index
    uint8_t GetIndex() const {
        for (std::size_t i = 0; i < card_map.size(); ++i) {
            if (card_map[i] == this) {
                return (uint8_t) i;
            }
        }
        // add new card
        assert(card_map.size() < 256);
        card_map.push_back(this);
        return (uint8_t) (card_map.size() - 1);
    }
    // constructor
    Card(std::string name_, uint8_t base_cost_, uint8_t cost_, const Card * upgraded_version_,
            CardFlagStruct flag_, std::initializer_list<Action> action_) :
            name(name_), base_cost(base_cost_), cost(cost_), upgraded_version(upgraded_version_),
            flag(flag_), action() {
        for (std::size_t i = 0; i < MAX_CARD_ACTIONS; ++i) {
            action[i].type = kActionNone;
        }
        std::copy(action_.begin(), action_.end(), action);
    }
};

//#define CREATE_CARD(variable_name, name, cost, upgraded_card, flags, actions) \
//const Card variable_name = {name, cost, cost, upgraded_card, flags, actions};

// common cards

const Card card_strike_plus = {
    "Strike+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .strike = 1, .starting = 1, .ironclad = 1, .silent = 1, .defect = 1, .watcher = 1},
    {{kActionAttack, 9, 1}, {kActionNone}}};
const Card card_strike = {
    "Strike", 1, 1, &card_strike_plus,
    {.attack = 1, .targeted = 1, .strike = 1, .starting = 1, .ironclad = 1, .silent = 1, .defect = 1, .watcher = 1},
    {{kActionAttack, 6, 1}, {kActionNone}}};

const Card card_defend_plus = {
    "Defend+", 1, 1, nullptr,
    {.skill = 1, .starting = 1, .ironclad = 1, .silent = 1, .defect = 1, .watcher = 1},
    {{kActionBlock, 8}, {kActionNone}}};
const Card card_defend = {
    "Defend", 1, 1, &card_defend_plus,
    {.skill = 1, .starting = 1, .ironclad = 1, .silent = 1, .defect = 1, .watcher = 1},
    {{kActionBlock, 5}, {kActionNone}}};

// curses

const Card card_ascenders_bane = {
    "Ascender's Bane", 0, 0, nullptr,
    {.curse = 1, .unplayable = 1, .ethereal = 1},
    {{kActionNone}}};

#include "cards_status.hpp"
#include "cards_colorless.hpp"
#include "cards_ironclad.hpp"
#include "cards_silent.hpp"
#include "cards_defect.hpp"
#include "cards_watcher.hpp"
