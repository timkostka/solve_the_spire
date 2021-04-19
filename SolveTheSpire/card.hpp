#pragma once

#include <cstdint>
#include <cassert>
#include <vector>

#include "orbs.hpp"
#include "action.hpp"
#include "buff_state.hpp"

//// card type
//enum CardType : uint8_t {
//    kCardTypeUnused = 0,
//    kCardTypeAttack,
//    kCardTypeSkill,
//    kCardTypePower,
//    kCardTypeCurse,
//};

struct Card;

// list of all cards in use
std::vector<const Card *> card_map;

// list of card flags
struct CardFlagStruct {
    unsigned int attack : 1;
    unsigned int skill : 1;
    unsigned int power : 1;
    unsigned int curse : 1;
    unsigned int status : 1;

    unsigned int unplayable : 1;

    unsigned int targeted : 1;

    unsigned int ethereal : 1;
    unsigned int exhausts : 1;
    // true if card name has "Strike"
    unsigned int strike : 1;
    unsigned int retain : 1;

    unsigned int starting : 1;
    unsigned int common : 1;
    unsigned int uncommon : 1;
    unsigned int rare : 1;
    unsigned int special : 1;

    unsigned int ironclad : 1;
    unsigned int silent : 1;
    unsigned int defect : 1;
    unsigned int watcher : 1;
    unsigned int colorless : 1;
};

static_assert(sizeof(CardFlagStruct) == 4, "");

// card flags
enum CardFlag : uint32_t {
    kCardFlagUnplayable = 1 << 0,
    kCardFlagTargeted = 1 << 1,
    kCardFlagEthereal = 1 << 2,
    kCardFlagExhausts = 1 << 3,
    // card has "Strike" in the name
    kCardFlagStrike = 1 << 4,
    kCardFlagRetain = 1 << 5,
    kCardFlagCommon = 1 << 6,
    kCardFlagUncommon = 1 << 7,
    kCardFlagRare = 1 << 8,
    kCardFlagNotOffered = 1 << 9,
    //kCardFlagInnate = 1 << 4,
};

// A Card holds information about a specific card.
struct Card {
    // text name as it appears
    std::string name;
    //CardType type;
    // base energy cost
    uint8_t base_cost;
    // current energy cost
    uint8_t cost;
    // card flags
    //uint32_t flags;
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
    //// return true if card is playable
    //inline bool IsUnplayable() const {
    //    return flag.unplayable;
    //}
    //// return true if card targets a single enemy
    //inline bool IsTargeted() const {
    //    return flag.targeted;
    //}
    //// return true if card is playable
    //inline bool Exhausts() const {
    //    return flag.exhausts;
    //}
    //// return true if card is playable
    //inline bool IsEthereal() const {
    //    return flag.ethereal;
    //}
};

const Card card_wound = {
    "Wound", 1, 1, nullptr,
    {.status = 1, .unplayable = 1},
    kActionNone};

// common cards

const Card card_strike_plus = {
    "Strike+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .strike = 1, .starting = 1, .ironclad = 1, .silent = 1, .defect = 1, .watcher = 1},
    {{kActionAttack, {9, 1}}, kActionNone}};
const Card card_strike = {
    "Strike", 1, 1, &card_strike_plus,
    {.attack = 1, .targeted = 1, .strike = 1, .starting = 1, .ironclad = 1, .silent = 1, .defect = 1, .watcher = 1},
    {{kActionAttack, {6, 1}}, kActionNone}};

const Card card_defend_plus = {
    "Defend+", 1, 1, nullptr,
    {.skill = 1, .starting = 1, .ironclad = 1, .silent = 1, .defect = 1, .watcher = 1},
    {{kActionBlock, 8}, kActionNone}};
const Card card_defend = {
    "Defend", 1, 1, &card_defend_plus,
    {.skill = 1, .starting = 1, .ironclad = 1, .silent = 1, .defect = 1, .watcher = 1},
    {{kActionBlock, 5}, kActionNone}};

// starting ironclad cards

const Card card_bash_plus = {
    "Bash+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .starting = 1, .ironclad = 1},
    {{kActionAttack, 10, 1}, {kActionDebuff, kBuffVulnerable, 3}, kActionNone}};
const Card card_bash = {
    "Bash", 2, 2, &card_bash_plus,
    {.attack = 1, .targeted = 1, .starting = 1, .ironclad = 1},
    {{kActionAttack, 8, 1}, {kActionDebuff, kBuffVulnerable, 2}, kActionNone}};

// common ironclad cards

const Card card_anger_plus = {
    "Anger+", 0, 0, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 8, 1}, {kActionAddCardToDiscardPile, card_anger_plus.GetIndex(), 1}, kActionNone}};
const Card card_anger = {
    "Anger", 0, 0, &card_anger_plus,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 6, 1}, {kActionAddCardToDiscardPile, card_anger.GetIndex(), 1}, kActionNone}};

// armaments

// body slam

// clash

const Card card_cleave_plus = {
    "Cleave+", 1, 1, nullptr,
    {.attack = 1, .common = 1, .ironclad = 1},
    {{kActionAttackAll, 11, 1}, kActionNone}};
const Card card_cleave = {
    "Cleave", 1, 1, &card_cleave_plus,
    {.attack = 1, .common = 1, .ironclad = 1},
    {{kActionAttackAll, 8, 1}, kActionNone}};

const Card card_clothesline_plus = {
    "Clothesline+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 14, 1}, {kActionDebuff, kBuffWeak, 3}, kActionNone}};
const Card card_clothesline = {
    "Clothesline", 2, 2, & card_clothesline_plus,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 12, 1}, {kActionDebuff, kBuffWeak, 2}, kActionNone}};

const Card card_flex_plus = {
    "Flex+", 1, 1, nullptr,
    {.skill = 1, .common = 1, .ironclad = 1},
    {{kActionBuff, kBuffStrength, 4}, {kActionBuff, kBuffStrengthDown, 4}, kActionNone}};
const Card card_flex = {
    "Flex", 1, 1, & card_flex_plus,
    {.skill = 1, .common = 1, .ironclad = 1},
    {{kActionBuff, kBuffStrength, 2}, {kActionBuff, kBuffStrengthDown, 2}, kActionNone}};

// havoc

// headbutt

const Card card_heavy_blade_plus = {
    "Heavy Blade+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttackHeavyBlade, 14, 5}, kActionNone}};
const Card card_heavy_blade = {
    "Heavy Blade", 2, 2, & card_heavy_blade_plus,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttackHeavyBlade, 14, 3}, kActionNone}};

const Card card_iron_wave_plus = {
    "Iron Wave+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 7, 1}, {kActionBlock, 7}, kActionNone}};
const Card card_iron_wave = {
    "Iron Wave", 1, 1, & card_iron_wave_plus,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 5, 1}, {kActionBlock, 5}, kActionNone}};

const Card card_perfected_strike_plus = {
    "Perfected Strike+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttackPerfectedStrike, 9, 3}, kActionNone}};
const Card card_perfected_strike = {
    "Perfected Strike", 2, 2, & card_perfected_strike_plus,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttackPerfectedStrike, 8, 2}, kActionNone}};

//const Card card_pommel_strike_plus = {
//    "Pommel Strike+", 1, 1, nullptr,
//    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
//    {{kActionAttack, 10, 1}, {kActionDrawCards, 2}, kActionNone}};
//const Card card_pommel_strike = {
//    "Pommel Strike", 1, 1, & card_pommel_strike_plus,
//    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
//    {{kActionAttack, 9, 1}, {kActionDrawCards, 1}, kActionNone}};
//
//const Card card_shrug_it_off_plus = {
//    "Shrug It Off+", 1, 1, nullptr,
//    {.skill = 1, .common = 1, .ironclad = 1},
//    {{kActionBlock, 11}, {kActionDrawCards, 1}, kActionNone}};
//const Card card_shrug_it_off = {
//    "Shrug It Off", 1, 1, & card_shrug_it_off_plus,
//    {.skill = 1, .common = 1, .ironclad = 1},
//    {{kActionBlock, 8}, {kActionDrawCards, 1}, kActionNone}};

const Card card_sword_boomerang_plus = {
    "Sword Boomerang+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 3, 4}, kActionNone}};
const Card card_sword_boomerang = {
    "Sword Boomerang", 1, 1, & card_sword_boomerang_plus,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 3, 3}, kActionNone}};

const Card card_thunderclap_plus = {
    "Thunderclap+", 1, 1, nullptr,
    {.attack = 1, .common = 1, .ironclad = 1},
    {{kActionAttackAll, 7, 1}, {kActionDebuffAll, kBuffVulnerable, 1}, kActionNone}};
const Card card_thunderclap = {
    "Thunderclap", 1, 1, & card_thunderclap_plus,
    {.attack = 1, .common = 1, .ironclad = 1},
    {{kActionAttackAll, 4, 1}, {kActionDebuffAll, kBuffVulnerable, 1}, kActionNone}};

// true grit

const Card card_twin_strike_plus = {
    "Twin Strike+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 7, 2}, kActionNone}};
const Card card_twin_strike = {
    "Twin Strike", 1, 1, & card_twin_strike_plus,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 5, 2}, kActionNone}};

// warcry

const Card card_wild_strike_plus = {
    "Wild Strike+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 17, 1}, {kActionAddCardToDrawPile, card_wound.GetIndex()}, kActionNone}};
const Card card_wild_strike = {
    "Wild Strike", 1, 1, & card_wild_strike_plus,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 12, 1}, {kActionAddCardToDrawPile, card_wound.GetIndex()}, kActionNone}};

// uncommon cards

const Card card_carnage_plus = {
    "Carnage+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .ethereal = 1, .uncommon = 1, .ironclad = 1},
    {{kActionAttack, 28, 1}, kActionNone}};
const Card card_carnage = {
    "Carnage", 2, 2, & card_carnage_plus,
    {.attack = 1, .targeted = 1, .ethereal = 1, .uncommon = 1, .ironclad = 1},
    {{kActionAttack, 20, 1}, kActionNone}};

const Card card_inflame_plus = {
    "Inflame+", 1, 1, nullptr,
    {.skill = 1, .uncommon = 1, .ironclad = 1},
    {{kActionBuff, kBuffStrength, 3}, kActionNone}};
const Card card_inflame = {
    "Inflame", 1, 1, & card_inflame_plus,
    {.skill = 1, .uncommon = 1, .ironclad = 1},
    {{kActionBuff, kBuffStrength, 2}, kActionNone}};

// curses

const Card card_ascenders_bane = {
    "Ascender's Bane", 0, 0, nullptr,
    {.curse = 1, .unplayable = 1, .ethereal = 1},
    {kActionNone}};

// starting silent cards

const Card card_neutralize_plus = {
    "Neutralize+", 0, 0, nullptr,
    {.attack = 1, .targeted = 1, .starting = 1, .silent = 1},
    {{kActionAttack, 4, 1}, {kActionDebuff, kBuffWeak, 2}, kActionNone}};
const Card card_neutralize = {
    "Neutralize", 0, 0, &card_neutralize_plus,
    {.attack = 1, .targeted = 1, .starting = 1, .silent = 1},
    {{kActionAttack, 3, 1}, {kActionDebuff, kBuffWeak, 1}, kActionNone}};

// TODO: add discard
const Card card_survivor_plus = {
    "Survivor+", 1, 1, nullptr,
    {.skill = 1, .starting = 1, .silent = 1},
    {{kActionBlock, 11, 1}, kActionNone}};
const Card card_survivor = {
    "Survivor", 1, 1, & card_survivor_plus,
    {.skill = 1, .starting = 1, .silent = 1},
    {{kActionBlock, 8, 1}, kActionNone}};

// starting defect cards

const Card card_zap_plus = {
    "Zap+", 0, 0, nullptr,
    {.skill = 1, .starting = 1, .defect = 1},
    {{kActionChannelOrb, kOrbLightning, 1}, kActionNone}};
const Card card_zap = {
    "Zap", 1, 1, & card_zap_plus,
    {.skill = 1, .starting = 1, .defect = 1},
    {{kActionChannelOrb, kOrbLightning, 1}, kActionNone}};

const Card card_dualcast_plus = {
    "Dualcast+", 0, 0, nullptr,
    {.skill = 1, .starting = 1, .defect = 1},
    {{kActionEvokeOrb, 2}, kActionNone}};
const Card card_dualcast = {
    "Dualcast", 1, 1, & card_dualcast_plus,
    {.skill = 1, .starting = 1, .defect = 1},
    {{kActionEvokeOrb, 2}, kActionNone}};

// starting watcher cards

const Card card_eruption_plus = {
    "Eruption+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .starting = 1, .watcher = 1},
    {{kActionAttack, 9, 1}, {kActionChangeStance, kStanceWrath}, kActionNone}};
const Card card_eruption = {
    "Eruption", 2, 2, & card_eruption_plus,
    {.attack = 1, .targeted = 1, .starting = 1, .watcher = 1},
    {{kActionAttack, 9, 1}, {kActionChangeStance, kStanceWrath}, kActionNone}};

const Card card_vigilance_plus = {
    "Vigilance+", 2, 2, nullptr,
    {.skill = 1, .starting = 1, .watcher = 1},
    {{kActionBlock, 12}, {kActionChangeStance, kStanceCalm}, kActionNone}};
const Card card_vigilance = {
    "Vigilance", 2, 2, & card_vigilance_plus,
    {.skill = 1, .starting = 1, .watcher = 1},
    {{kActionBlock, 8}, {kActionChangeStance, kStanceCalm}, kActionNone}};

const Card card_miracle_plus = {
    "Miracle+", 0, 0, nullptr,
    {.skill = 1, .special = 1, .watcher = 1},
    {{kActionGainEnergy, 2}, kActionNone}};
const Card card_miracle = {
    "Miracle", 0, 0, &card_miracle_plus,
    {.skill = 1, .special = 1, .watcher = 1},
    {{kActionGainEnergy, 1}, kActionNone}};

// common watcher cards

const Card card_bowling_bash_plus = {
    "Bowling Bash+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttackBowlingBash, 10, 1}, kActionNone}};
const Card card_bowling_bash = {
    "Bowling Bash", 1, 1, & card_bowling_bash_plus,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttackBowlingBash, 7, 1}, kActionNone}};

const Card card_consecrate_plus = {
    "Consecrate+", 1, 1, nullptr,
    {.attack = 1, .common = 1, .watcher = 1},
    {{kActionAttackAll, 8, 1}, kActionNone}};
const Card card_consecrate = {
    "Consecrate", 1, 1, & card_consecrate_plus,
    {.attack = 1, .common = 1, .watcher = 1},
    {{kActionAttackAll, 5, 1}, kActionNone}};

const Card card_crescendo_plus = {
    "Crescendo+", 0, 0, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionChangeStance, kStanceWrath}, kActionNone}};
const Card card_crescendo = {
    "Crescendo", 1, 1, &card_crescendo_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionChangeStance, kStanceWrath}, kActionNone}};

const Card card_crush_joints_plus = {
    "Crush Joints+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 10, 1}, {kActionLastCardSkill}, {kActionDebuff, kBuffVulnerable, 2}, kActionNone}};
const Card card_crush_joints = {
    "Crush Joints", 1, 1, & card_crush_joints_plus,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 8, 1}, {kActionLastCardSkill}, {kActionDebuff, kBuffVulnerable, 1}, kActionNone}};

// Cut Through Fate
//const Card card_cut_through_fate_plus = {
//    "Cut Through Fate+", 1, 1, nullptr,
//    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
//    {{kActionAttack, 9, 1}, {kActionScry, 3}, {kActionDrawCards, 1}, kActionNone}};
//const Card card_cut_through_fate = {
//    "Cut Through Fate", 1, 1, & card_cut_through_fate_plus,
//    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
//    {{kActionAttack, 7, 1}, {kActionScry, 2}, {kActionDrawCards, 1}, kActionNone}};

const Card card_empty_body_plus = {
    "Empty Body+", 1, 1, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 10}, {kActionChangeStance, kStanceNone}, kActionNone}};
const Card card_empty_body = {
    "Empty Body", 1, 1, & card_empty_body_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 7}, {kActionChangeStance, kStanceNone}, kActionNone}};

const Card card_empty_fist_plus = {
    "Empty Fist+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 14, 1}, {kActionChangeStance, kStanceNone}, kActionNone}};
const Card card_empty_fist = {
    "Empty Fist", 1, 1, & card_empty_fist_plus,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 9, 1}, {kActionChangeStance, kStanceNone}, kActionNone}};

//const Card card_insight_plus = {
//    "Insight+", 0, 0, nullptr,
//    {.skill = 1, .retain = 1, .special = 1, .watcher = 1},
//    {{kActionDrawCards, 3}, kActionNone}};
//const Card card_insight = {
//    "Insight", 0, 0, & card_insight_plus,
//    {.skill = 1, .retain = 1, .special = 1, .watcher = 1},
//    {{kActionDrawCards, 2}, kActionNone}};
//
//const Card card_evaluate_plus = {
//    "Evaluate+", 1, 1, nullptr,
//    {.skill = 1, .common = 1, .watcher = 1},
//    {{kActionBlock, 10}, {kActionAddCardToDrawPile, card_insight.GetIndex()}, kActionNone}};
//const Card card_evaluate = {
//    "Evaluate", 1, 1, & card_evaluate_plus,
//    {.skill = 1, .common = 1, .watcher = 1},
//    {{kActionBlock, 6}, {kActionAddCardToDrawPile, card_insight.GetIndex()}, kActionNone}};

const Card card_flurry_of_blows_plus = {
    "Flurry of Blows+", 0, 0, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 6, 0}, kActionNone}};
const Card card_flurry_of_blows = {
    "Flurry of Blows", 0, 0, & card_flurry_of_blows_plus,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 4, 0}, kActionNone}};

const Card card_flying_sleeves_plus = {
    "Flying Sleeves+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .retain = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 6, 2}, kActionNone}};
const Card card_flying_sleeves = {
    "Flying Sleeves", 1, 1, & card_flying_sleeves_plus,
    {.attack = 1, .targeted = 1, .retain = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 4, 2}, kActionNone}};

const Card card_follow_up_plus = {
    "Follow Up+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .retain = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 11, 1}, {kActionLastCardAttack}, {kActionGainEnergy, 1}, kActionNone}};
const Card card_follow_up = {
    "Follow Up", 1, 1, &card_follow_up_plus,
    {.attack = 1, .targeted = 1, .retain = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 7, 1}, {kActionLastCardAttack}, {kActionGainEnergy, 1}, kActionNone}};

const Card card_halt_plus = {
    "Halt+", 0, 0, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 10}, {kActionChangeStance, kStanceNone}, kActionNone}};
const Card card_halt = {
    "Halt", 0, 0, &card_halt_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 7}, {kActionInWrath}, {kActionBlock, 7}, kActionNone}};

const Card card_just_lucky_plus = {
    "Just Lucky+", 0, 0, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionScry, 2}, {kActionBlock, 3}, {kActionAttack, 4}, kActionNone}};
const Card card_just_lucky = {
    "Just Lucky", 0, 0, & card_just_lucky_plus,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionScry, 1}, {kActionBlock, 2}, {kActionAttack, 3}, kActionNone}};

// Pressure Points

const Card card_prostrate_plus = {
    "Prostrate+", 0, 0, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionGainMantra, 3}, {kActionBlock, 4}, kActionNone}};
const Card card_prostrate = {
    "Prostrate", 0, 0, & card_prostrate_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionGainMantra, 2}, {kActionBlock, 4}, kActionNone}};


const Card card_protect_plus = {
    "Protect+", 2, 2, nullptr,
    {.skill = 1, .retain = 1, .common = 1, .watcher = 1},
    {{kActionChangeStance, kStanceCalm}, kActionNone}};
const Card card_protect = {
    "Protect", 2, 2, & card_protect_plus,
    {.skill = 1, .retain = 1, .common = 1, .watcher = 1},
    {{kActionChangeStance, kStanceCalm}, kActionNone}};

const Card card_sash_whip_plus = {
    "Sash Whip+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 10, 1}, {kActionLastCardAttack}, {kActionDebuff, kBuffWeak, 2}, kActionNone}};
const Card card_sash_whip = {
    "Sash Whip", 1, 1, &card_sash_whip_plus,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 8, 1}, {kActionLastCardAttack}, {kActionDebuff, kBuffWeak, 1}, kActionNone}};

const Card card_third_eye_plus = {
    "Third Eye+", 1, 1, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 9}, {kActionScry, 3}, kActionNone}};
const Card card_third_eye = {
    "Third Eye", 1, 1, & card_third_eye_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 7}, {kActionScry, 3}, kActionNone}};

const Card card_tranquility_plus = {
    "Tranquility+", 0, 0, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionChangeStance, kStanceCalm}, kActionNone}};
const Card card_tranquility = {
    "Tranquility", 1, 1, & card_tranquility_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionChangeStance, kStanceCalm}, kActionNone}};
