#pragma once
#pragma once

#include "cards.hpp"

// starting watcher cards

const Card card_eruption_plus = {
    "Eruption+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .starting = 1, .watcher = 1},
    {{kActionAttack, 9, 1}, {kActionChangeStance, kStanceWrath}, {kActionNone}}};
const Card card_eruption = {
    "Eruption", 2, 2, &card_eruption_plus,
    {.attack = 1, .targeted = 1, .starting = 1, .watcher = 1},
    {{kActionAttack, 9, 1}, {kActionChangeStance, kStanceWrath}, {kActionNone}}};

const Card card_vigilance_plus = {
    "Vigilance+", 2, 2, nullptr,
    {.skill = 1, .starting = 1, .watcher = 1},
    {{kActionBlock, 12}, {kActionChangeStance, kStanceCalm}, {kActionNone}}};
const Card card_vigilance = {
    "Vigilance", 2, 2, &card_vigilance_plus,
    {.skill = 1, .starting = 1, .watcher = 1},
    {{kActionBlock, 8}, {kActionChangeStance, kStanceCalm}, {kActionNone}}};

const Card card_miracle_plus = {
    "Miracle+", 0, 0, nullptr,
    {.skill = 1, .special = 1, .watcher = 1},
    {{kActionGainEnergy, 2}, {kActionNone}}};
const Card card_miracle = {
    "Miracle", 0, 0, &card_miracle_plus,
    {.skill = 1, .special = 1, .watcher = 1},
    {{kActionGainEnergy, 1}, {kActionNone}}};

// created watcher cards

const Card card_smite_plus = {
    "Smite+", 1, 1, nullptr,
    {.attack = 1, .exhausts = 1, .special = 1, .watcher = 1},
    {{kActionAttack, 16, 1}, {kActionNone}}};
const Card card_smite = {
    "Smite", 1, 1, &card_smite_plus,
    {.attack = 1, .exhausts = 1, .special = 1, .watcher = 1},
    {{kActionAttack, 12, 1}, {kActionNone}}};

const Card card_through_violence_plus = {
    "Through Violence+", 0, 0, nullptr,
    {.attack = 1, .exhausts = 1, .retain = 1, .special = 1, .watcher = 1},
    {{kActionAttack, 30, 1}, {kActionNone}}};
const Card card_through_violence = {
    "Through Violence", 0, 0, &card_through_violence_plus,
    {.attack = 1, .exhausts = 1, .retain = 1, .special = 1, .watcher = 1},
    {{kActionAttack, 40, 1}, {kActionNone}}};

// common watcher cards

const Card card_bowling_bash_plus = {
    "Bowling Bash+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttackBowlingBash, 10, 1}, {kActionNone}}};
const Card card_bowling_bash = {
    "Bowling Bash", 1, 1, &card_bowling_bash_plus,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttackBowlingBash, 7, 1}, {kActionNone}}};

const Card card_consecrate_plus = {
    "Consecrate+", 1, 1, nullptr,
    {.attack = 1, .common = 1, .watcher = 1},
    {{kActionAttackAll, 8, 1}, {kActionNone}}};
const Card card_consecrate = {
    "Consecrate", 1, 1, &card_consecrate_plus,
    {.attack = 1, .common = 1, .watcher = 1},
    {{kActionAttackAll, 5, 1}, {kActionNone}}};

const Card card_crescendo_plus = {
    "Crescendo+", 0, 0, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionChangeStance, kStanceWrath}, {kActionNone}}};
const Card card_crescendo = {
    "Crescendo", 1, 1, &card_crescendo_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionChangeStance, kStanceWrath}, {kActionNone}}};

const Card card_crush_joints_plus = {
    "Crush Joints+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 10, 1}, {kActionLastCardSkill}, {kActionDebuff, kBuffVulnerable, 2}, {kActionNone}}};
const Card card_crush_joints = {
    "Crush Joints", 1, 1, &card_crush_joints_plus,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 8, 1}, {kActionLastCardSkill}, {kActionDebuff, kBuffVulnerable, 1}, {kActionNone}}};

// Cut Through Fate
//const Card card_cut_through_fate_plus = {
//    "Cut Through Fate+", 1, 1, nullptr,
//    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
//    {{kActionAttack, 9, 1}, {kActionScry, 3}, {kActionDrawCards, 1}, {kActionNone}}};
//const Card card_cut_through_fate = {
//    "Cut Through Fate", 1, 1, & card_cut_through_fate_plus,
//    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
//    {{kActionAttack, 7, 1}, {kActionScry, 2}, {kActionDrawCards, 1}, {kActionNone}}};

const Card card_empty_body_plus = {
    "Empty Body+", 1, 1, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 10}, {kActionChangeStance, kStanceNone}, {kActionNone}}};
const Card card_empty_body = {
    "Empty Body", 1, 1, &card_empty_body_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 7}, {kActionChangeStance, kStanceNone}, {kActionNone}}};

const Card card_empty_fist_plus = {
    "Empty Fist+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 14, 1}, {kActionChangeStance, kStanceNone}, {kActionNone}}};
const Card card_empty_fist = {
    "Empty Fist", 1, 1, &card_empty_fist_plus,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 9, 1}, {kActionChangeStance, kStanceNone}, {kActionNone}}};

const Card card_insight_plus = {
    "Insight+", 0, 0, nullptr,
    {.skill = 1, .retain = 1, .special = 1, .watcher = 1},
    {{kActionDrawCards, 3}, {kActionNone}}};
const Card card_insight = {
    "Insight", 0, 0, &card_insight_plus,
    {.skill = 1, .retain = 1, .special = 1, .watcher = 1},
    {{kActionDrawCards, 2}, {kActionNone}}};

const Card card_evaluate_plus = {
    "Evaluate+", 1, 1, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 10}, {kActionAddCardToDrawPile, card_insight.GetIndex(), 1}, {kActionNone}}};
const Card card_evaluate = {
    "Evaluate", 1, 1, &card_evaluate_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 6}, {kActionAddCardToDrawPile, card_insight.GetIndex(), 1}, {kActionNone}}};

const Card card_flurry_of_blows_plus = {
    "Flurry of Blows+", 0, 0, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 6, 0}, {kActionNone}}};
const Card card_flurry_of_blows = {
    "Flurry of Blows", 0, 0, &card_flurry_of_blows_plus,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 4, 0}, {kActionNone}}};

const Card card_flying_sleeves_plus = {
    "Flying Sleeves+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .retain = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 6, 2}, {kActionNone}}};
const Card card_flying_sleeves = {
    "Flying Sleeves", 1, 1, &card_flying_sleeves_plus,
    {.attack = 1, .targeted = 1, .retain = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 4, 2}, {kActionNone}}};

const Card card_follow_up_plus = {
    "Follow Up+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .retain = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 11, 1}, {kActionLastCardAttack}, {kActionGainEnergy, 1}, {kActionNone}}};
const Card card_follow_up = {
    "Follow Up", 1, 1, &card_follow_up_plus,
    {.attack = 1, .targeted = 1, .retain = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 7, 1}, {kActionLastCardAttack}, {kActionGainEnergy, 1}, {kActionNone}}};

const Card card_halt_plus = {
    "Halt+", 0, 0, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 10}, {kActionChangeStance, kStanceNone}, {kActionNone}}};
const Card card_halt = {
    "Halt", 0, 0, &card_halt_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 7}, {kActionInWrath}, {kActionBlock, 7}, {kActionNone}}};

const Card card_just_lucky_plus = {
    "Just Lucky+", 0, 0, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionScry, 2}, {kActionBlock, 3}, {kActionAttack, 4}, {kActionNone}}};
const Card card_just_lucky = {
    "Just Lucky", 0, 0, &card_just_lucky_plus,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionScry, 1}, {kActionBlock, 2}, {kActionAttack, 3}, {kActionNone}}};

// Pressure Points

const Card card_prostrate_plus = {
    "Prostrate+", 0, 0, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionGainMantra, 3}, {kActionBlock, 4}, {kActionNone}}};
const Card card_prostrate = {
    "Prostrate", 0, 0, &card_prostrate_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionGainMantra, 2}, {kActionBlock, 4}, {kActionNone}}};

const Card card_protect_plus = {
    "Protect+", 2, 2, nullptr,
    {.skill = 1, .retain = 1, .common = 1, .watcher = 1},
    {{kActionChangeStance, kStanceCalm}, {kActionNone}}};
const Card card_protect = {
    "Protect", 2, 2, &card_protect_plus,
    {.skill = 1, .retain = 1, .common = 1, .watcher = 1},
    {{kActionChangeStance, kStanceCalm}, {kActionNone}}};

const Card card_sash_whip_plus = {
    "Sash Whip+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 10, 1}, {kActionLastCardAttack}, {kActionDebuff, kBuffWeak, 2}, {kActionNone}}};
const Card card_sash_whip = {
    "Sash Whip", 1, 1, &card_sash_whip_plus,
    {.attack = 1, .targeted = 1, .common = 1, .watcher = 1},
    {{kActionAttack, 8, 1}, {kActionLastCardAttack}, {kActionDebuff, kBuffWeak, 1}, {kActionNone}}};

const Card card_third_eye_plus = {
    "Third Eye+", 1, 1, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 9}, {kActionScry, 3}, {kActionNone}}};
const Card card_third_eye = {
    "Third Eye", 1, 1, &card_third_eye_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionBlock, 7}, {kActionScry, 3}, {kActionNone}}};

const Card card_tranquility_plus = {
    "Tranquility+", 0, 0, nullptr,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionChangeStance, kStanceCalm}, {kActionNone}}};
const Card card_tranquility = {
    "Tranquility", 1, 1, &card_tranquility_plus,
    {.skill = 1, .common = 1, .watcher = 1},
    {{kActionChangeStance, kStanceCalm}, {kActionNone}}};

// uncommon Watcher cards

// battle hymn

const Card card_carve_reality_plus = {
    "Carve Reality+", 1, 1, nullptr,
    {.attack = 1, .uncommon = 1, .watcher = 1},
    {{kActionAttack, 10, 1}, {kActionAddCardToHand, card_smite.GetIndex(), 1}, {kActionNone}}};
const Card card_carve_reality = {
    "Carve Reality", 1, 1, &card_carve_reality_plus,
    {.attack = 1, .uncommon = 1, .watcher = 1},
    {{kActionAttack, 6, 1}, {kActionAddCardToHand, card_smite.GetIndex(), 1}, {kActionNone}}};

// collect
// conclude
// deceive reality
// empty mind
// fasting
// fear no evil
// foreign influence
// foresight
// indignation
// inner peace
// like water
// meditate
// mental fortress
// nirvana
// perseverance
// pray

const Card card_reach_heaven_plus = {
    "Reach Heaven+", 2, 2, nullptr,
    {.attack = 1, .uncommon = 1, .watcher = 1},
    {{kActionAttack, 15, 1}, {kActionAddCardToDrawPile, card_through_violence.GetIndex(), 1}, {kActionNone}}};
const Card card_reach_heaven = {
    "Reach Heaven", 1, 1, &card_reach_heaven_plus,
    {.attack = 1, .uncommon = 1, .watcher = 1},
    {{kActionAttack, 10, 1}, {kActionAddCardToDrawPile, card_through_violence.GetIndex(), 1}, {kActionNone}}};

// rushdown
// sanctity
// sands of time
// simmering fury
// study
// swivel
// talk to the hand
// tantrum
// wallop
// wave of the hand
// weave
// wheel kick
// windmill strike
// worship
// wreath of flame
