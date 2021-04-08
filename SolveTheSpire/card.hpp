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

// card flags
enum CardFlag : uint8_t {
    kCardFlagUnplayable = 1 << 0,
    kCardFlagTargeted = 1 << 1,
    kCardFlagEthereal = 1 << 2,
    kCardFlagExhausts = 1 << 3,
    //kCardFlagInnate = 1 << 4,
};

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
    // card flags
    uint32_t flags;
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
    // return true if card is playable
    inline bool IsUnplayable() const {
        return flags & kCardFlagUnplayable;
    }
    // return true if card targets a single enemy
    inline bool IsTargeted() const {
        return flags & kCardFlagTargeted;
    }
    // return true if card is playable
    inline bool Exhausts() const {
        return flags & kCardFlagExhausts;
    }
    // return true if card is playable
    inline bool IsEthereal() const {
        return flags & kCardFlagEthereal;
    }
};

const Card card_wound = {
    "Wound", kCardTypeCurse, 1, 1, kCardFlagUnplayable, nullptr,
    kActionNone};

const Card card_strike_plus = {
    "Strike+", kCardTypeAttack, 1, 1, kCardFlagTargeted, nullptr,
    {{kActionAttack, {9, 1}}, kActionNone}};
const Card card_strike = {
    "Strike", kCardTypeAttack, 1, 1, kCardFlagTargeted, &card_strike_plus,
    {{kActionAttack, {6, 1}}, kActionNone}};

const Card card_defend_plus = {
    "Defend+", kCardTypeSkill, 1, 1, 0, nullptr,
    {{kActionBlock, {8}}, kActionNone}};
const Card card_defend = {
    "Defend", kCardTypeSkill, 1, 1, 0, &card_defend_plus,
    {{kActionBlock, {5}}, kActionNone}};

const Card card_bash_plus = {
    "Bash+", kCardTypeAttack, 2, 2, kCardFlagTargeted, nullptr,
    {{kActionAttack, {10, 1}}, {kActionDebuff, {kBuffVulnerable, 3}}, kActionNone}};
const Card card_bash = {
    "Bash", kCardTypeAttack, 2, 2, kCardFlagTargeted, &card_bash_plus,
    {{kActionAttack, {8, 1}}, {kActionDebuff, {kBuffVulnerable, 2}}, kActionNone}};

const Card card_anger_plus = {
    "Anger+", kCardTypeAttack, 0, 0, kCardFlagTargeted, nullptr,
    {{kActionAttack, {8, 1}}, {kActionAddCardToDiscardPile, card_anger_plus.GetIndex()}, kActionNone}};
const Card card_anger = {
    "Anger", kCardTypeAttack, 0, 0, kCardFlagTargeted, &card_anger_plus,
    {{kActionAttack, {6, 1}}, {kActionAddCardToDiscardPile, card_anger.GetIndex()}, kActionNone}};

// armaments

// body slam

// clash

const Card card_cleave_plus = {
    "Cleave+", kCardTypeAttack, 1, 1, 0, nullptr,
    {{kActionAttackAll, {8, 1}}, kActionNone}};
const Card card_cleave = {
    "Cleave", kCardTypeAttack, 1, 1, 0, &card_cleave_plus,
    {{kActionAttackAll, {11, 1}}, kActionNone}};

const Card card_clothesline_plus = {
    "Clothesline+", kCardTypeAttack, 1, 1, 0, nullptr,
    {{kActionAttack, 14, 1}, {kActionDebuff, {kBuffWeak, 3}}, kActionNone}};
const Card card_clothesline = {
    "Clothesline", kCardTypeAttack, 1, 1, kCardFlagTargeted, &card_clothesline_plus,
    {{kActionAttack, 12, 1}, {kActionDebuff, {kBuffWeak, 2}}, kActionNone}};

const Card card_flex_plus = {
    "Flex+", kCardTypeSkill, 1, 1, 0, nullptr,
    {{kActionBuff, kBuffStrength, 4}, {kActionBuff, kBuffStrengthDown, 4}, kActionNone}};
const Card card_flex = {
    "Flex", kCardTypeSkill, 1, 1, 0, &card_flex_plus,
    {{kActionBuff, kBuffStrength, 2}, {kActionBuff, kBuffStrengthDown, 2}, kActionNone}};

// havoc

// headbutt

// heavy blade

const Card card_iron_wave_plus = {
    "Iron Wave+", kCardTypeAttack, 1, 1, kCardFlagTargeted, nullptr,
    {{kActionAttack, 7, 1}, {kActionBlock, 7}, kActionNone}};
const Card card_iron_wave = {
    "Iron Wave", kCardTypeAttack, 1, 1, kCardFlagTargeted, &card_iron_wave_plus,
    {{kActionAttack, 5, 1}, {kActionBlock, 5}, kActionNone}};

// perfected strike

const Card card_pommel_strike_plus = {
    "Pommel Strike+", kCardTypeAttack, 1, 1, kCardFlagTargeted, nullptr,
    {{kActionAttack, {10, 1}}, {kActionDrawCards, 2}, kActionNone}};
const Card card_pommel_strike = {
    "Pommel Strike", kCardTypeAttack, 1, 1, kCardFlagTargeted, &card_pommel_strike_plus,
    {{kActionAttack, {9, 1}}, {kActionDrawCards, 1}, kActionNone}};

const Card card_shrug_it_off_plus = {
    "Shrug It Off+", kCardTypeSkill, 1, 1, 0, nullptr,
    {{kActionBlock, 11}, {kActionDrawCards, 1}, kActionNone}};
const Card card_shrug_it_off = {
    "Shrug It Off", kCardTypeSkill, 1, 1, 0, &card_shrug_it_off_plus,
    {{kActionBlock, 8}, {kActionDrawCards, 1}, kActionNone}};

const Card card_sword_boomerang_plus = {
    "Sword Boomerang+", kCardTypeAttack, 1, 1, kCardFlagTargeted, nullptr,
    {{kActionAttack, {3, 4}}, kActionNone}};
const Card card_sword_boomerang = {
    "Sword Boomerang", kCardTypeAttack, 1, 1, kCardFlagTargeted, &card_sword_boomerang_plus,
    {{kActionAttack, {3, 3}}, kActionNone}};

const Card card_thunderclap_plus = {
    "Thunderclap+", kCardTypeAttack, 1, 1, 0, nullptr,
    {{kActionAttackAll, {7, 1}}, {kActionDebuffAll, {kBuffVulnerable, 1}}, kActionNone}};
const Card card_thunderclap = {
    "Thunderclap", kCardTypeAttack, 1, 1, 0, & card_thunderclap_plus,
    {{kActionAttackAll, {4, 1}}, {kActionDebuffAll, {kBuffVulnerable, 1}}, kActionNone}};

// true grit

const Card card_twin_strike = {
    "Twin Strike", kCardTypeAttack, 1, 1, kCardFlagTargeted, nullptr,
    {{kActionAttack, {5, 2}}, kActionNone}};

// warcry

const Card card_wild_strike_plus = {
    "Wild Strike+", kCardTypeAttack, 1, 1, kCardFlagTargeted, nullptr,
    {{kActionAttack, {17, 1}}, {kActionAddCardToDrawPile, card_wound.GetIndex()}, kActionNone}};
const Card card_wild_strike = {
    "Wild Strike", kCardTypeAttack, 1, 1, kCardFlagTargeted, &card_wild_strike_plus,
    {{kActionAttack, {12, 1}}, {kActionAddCardToDrawPile, card_wound.GetIndex()}, kActionNone}};

// uncommon cards

const Card card_carnage_plus = {
    "Carnage+", kCardTypeAttack, 2, 2, kCardFlagTargeted, nullptr,
    {{kActionAttack, {28, 1}}, kActionNone}};
const Card card_carnage = {
    "Carnage", kCardTypeAttack, 2, 2, kCardFlagTargeted, &card_carnage_plus,
    {{kActionAttack, {20, 1}}, kActionNone}};


const Card card_inflame_plus = {
    "Inflame+", kCardTypeSkill, 1, 1, 0, nullptr,
    {{kActionBuff, kBuffStrength, 3}, kActionNone}};
const Card card_inflame = {
    "Inflame", kCardTypeSkill, 1, 1, 0, &card_inflame_plus,
    {{kActionBuff, kBuffStrength, 2}, kActionNone}};
