#pragma once

#include <cstdint>
#include <cassert>
#include <vector>

#include "action.hpp"
#include "buff_state.hpp"

// card type
enum CardType : uint8_t {
    kCardTypeUnused = 0,
    kCardTypeAttack,
    kCardTypeSkill,
    kCardTypePower,
    kCardTypeCurse,
};

struct Card;

// list of all cards in use
std::vector<const Card *> card_index;

// A Card holds information about a specific card.
struct Card {
    // text name as it appears
    const char * name;
    // card type
    CardType type;
    // base energy cost
    uint8_t base_cost;
    // current energy cost
    uint8_t cost;
    // true if card has a single enemy target (e.g. Strike)
    bool targeted;
    // true if card is ethereal
    bool ethereal;
    // true if card exhausts
    bool exhausts;
    // true if card is innate
    bool innate;
    // upgraded card version, or nullptr
    const Card * upgraded_version;
    // list of actions
    Action action[5];
    // card index
    uint8_t GetIndex() const {
        for (std::size_t i = 0; i < card_index.size(); ++i) {
            if (card_index[i] == this) {
                return (uint8_t) i;
            }
        }
        // add new card
        assert(card_index.size() < 256);
        card_index.push_back(this);
        return (uint8_t) (card_index.size() - 1);
    }
    // return true if card targets a single enemy
    bool IsTargeted() const {
        return targeted;
    }
};

const Card card_strike_plus = {"Strike+", kCardTypeAttack, 1, 1, true, false, false, false, nullptr, {{kActionAttack, {9, 1}}}};
const Card card_strike = {"Strike", kCardTypeAttack, 1, 1, true, false, false, false, &card_strike_plus, {{kActionAttack, {6, 1}}}};

const Card card_defend_plus = {"Defend+", kCardTypeSkill, 1, 1, false, false, false, false, nullptr, {{kActionBlock, {8}}}};
const Card card_defend = {"Defend", kCardTypeSkill, 1, 1, false, false, false, false, &card_defend_plus, {{kActionBlock, {5}}}};

const Card card_bash_plus = {"Bash+", kCardTypeAttack, 2, 2, true, false, false, false, nullptr, {{kActionAttack, {10, 1}}, {kActionDebuff, {kBuffVulnerable, 3}}}};
const Card card_bash = {"Bash", kCardTypeAttack, 2, 2, true, false, false, false, &card_bash_plus, {{kActionAttack, {8, 1}}, {kActionDebuff, {kBuffVulnerable, 2}}}};

const Card card_twin_strike = {"Twin Strike", kCardTypeAttack, 1, 1, true, false, false, false, nullptr,
    {{kActionAttack, {5, 2}}}};

const Card card_thunderclap = {"Thunderclap", kCardTypeAttack, 1, 1, true, false, false, false, nullptr,
    {{kActionAttackAll, {4, 1}}, {kActionDebuffAll, {kBuffVulnerable, 1}}}};
