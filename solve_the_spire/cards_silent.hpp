#pragma once

#include "cards.hpp"

///////////////////////////
// starting silent cards //
///////////////////////////

const Card card_neutralize_plus = {
    "Neutralize+", 0, 0, nullptr,
    {.attack = 1, .targeted = 1, .starting = 1, .silent = 1},
    {{kActionAttack, 4, 1}, {kActionDebuff, kBuffWeak, 2}, {kActionNone}}};
const Card card_neutralize = {
    "Neutralize", 0, 0, &card_neutralize_plus,
    {.attack = 1, .targeted = 1, .starting = 1, .silent = 1},
    {{kActionAttack, 3, 1}, {kActionDebuff, kBuffWeak, 1}, {kActionNone}}};

// TODO: add discard
const Card card_survivor_plus = {
    "Survivor+", 1, 1, nullptr,
    {.skill = 1, .starting = 1, .silent = 1},
    {{kActionBlock, 11, 1}, {kActionNone}}};
const Card card_survivor = {
    "Survivor", 1, 1, &card_survivor_plus,
    {.skill = 1, .starting = 1, .silent = 1},
    {{kActionBlock, 8, 1}, {kActionNone}}};

/////////////////////////
// common silent cards //
/////////////////////////

// acrobatics

const Card card_backflip_plus = {
    "Backflip+", 1, 1, nullptr,
    {.skill = 1, .common = 1, .silent = 1},
    {{kActionBlock, 8, 1}, {kActionDrawCards, 2}, {kActionNone}}};
const Card card_backflip = {
    "Backflip", 1, 1, &card_backflip_plus,
    {.skill = 1, .common = 1, .silent = 1},
    {{kActionBlock, 5, 1}, {kActionDrawCards, 2}, {kActionNone}}};

const Card card_bane_plus = {
    "Bane+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .silent = 1},
    {{kActionAttack, 10, 1}, {kActionIfPoisoned}, {kActionAttack, 10, 1}, {kActionNone}}};
const Card card_bane = {
    "Bane", 1, 1, &card_bane_plus,
    {.attack = 1, .targeted = 1, .common = 1, .silent = 1},
    {{kActionAttack, 7, 1}, {kActionIfPoisoned}, {kActionAttack, 7, 1}, {kActionNone}}};

// blade dance

// cloak and dagger

const Card card_dagger_spray_plus = {
    "Dagger Spray+", 0, 0, nullptr,
    {.attack = 1, .common = 1, .silent = 1},
    {{kActionAttackAll, 6, 2}, {kActionNone}}};
const Card card_dagger_spray = {
    "Dagger Spray", 0, 0, &card_dagger_spray_plus,
    {.attack = 1, .common = 1, .silent = 1},
    {{kActionAttackAll, 4, 2}, {kActionNone}}};

// dagger throw

const Card card_deadly_poison_plus = {
    "Deadly Poison+", 1, 1, nullptr,
    {.skill = 1, .targeted = 1, .common = 1, .silent = 1},
    {{kActionDebuff, kBuffPoison, 7}, {kActionNone}}};
const Card card_deadly_poison = {
    "Deadly Poison", 1, 1, &card_deadly_poison_plus,
    {.skill = 1, .targeted = 1, .common = 1, .silent = 1},
    {{kActionDebuff, kBuffPoison, 5}, {kActionNone}}};

const Card card_deflect_plus = {
    "Deflect+", 0, 0, nullptr,
    {.skill = 1, .common = 1, .silent = 1},
    {{kActionBlock, 7}, {kActionNone}}};
const Card card_deflect = {
    "Deflect", 0, 0, &card_deflect_plus,
    {.skill = 1, .common = 1, .silent = 1},
    {{kActionBlock, 4}, {kActionNone}}};

// dodge and roll

// flying knee

// outmaneuver

// piercing wail

const Card card_poisoned_stab_plus = {
    "Poisoned Stab+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .silent = 1},
    {{kActionAttack, 8, 1}, {kActionDebuff, kBuffPoison, 4}, {kActionNone}}};
const Card card_poisoned_stab = {
    "Poisoned Stab", 1, 1, &card_poisoned_stab_plus,
    {.attack = 1, .targeted = 1, .common = 1, .silent = 1},
    {{kActionAttack, 6, 1}, {kActionDebuff, kBuffPoison, 3}, {kActionNone}}};

// prepared

const Card card_quick_slash_plus = {
    "Quick Slash+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .silent = 1},
    {{kActionAttack, 12, 1}, {kActionDrawCards, 2}, {kActionNone}}};
const Card card_quick_slash = {
    "Quick Slash", 1, 1, &card_quick_slash_plus,
    {.attack = 1, .targeted = 1, .common = 1, .silent = 1},
    {{kActionAttack, 8, 1}, {kActionDrawCards, 1}, {kActionNone}}};

const Card card_slice_plus = {
    "Slice+", 0, 0, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .silent = 1},
    {{kActionAttack, 9, 1}, {kActionNone}}};
const Card card_slice = {
    "Slice", 0, 0, &card_slice,
    {.attack = 1, .targeted = 1, .common = 1, .silent = 1},
    {{kActionAttack, 6, 1}, {kActionNone}}};

// sneaky strike

const Card card_sucker_punch_plus = {
    "Sucker Punch+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .silent = 1},
    {{kActionAttack, 9, 1}, {kActionDebuff, kBuffWeak, 2}, {kActionNone}}};
const Card card_sucker_punch = {
    "Sucker Punch", 1, 1, &card_sucker_punch_plus,
    {.attack = 1, .targeted = 1, .common = 1, .silent = 1},
    {{kActionAttack, 7, 1}, {kActionDebuff, kBuffWeak, 1}, {kActionNone}}};

///////////////////////////
// uncommon silent cards //
///////////////////////////

// accuracy
// all-out attack
// backstab
// blur
// bouncing flask
// calculated gamble
// caltrops
// catalyst
// choke
// concentrate
// crippling cloud

const Card card_crippling_cloud_plus = {
    "Crippling Cloud+", 2, 2, nullptr,
    {.skill = 1, .uncommon = 1, .silent = 1},
    {{kActionDebuff, kBuffPoison, 7}, {kActionDebuff, kBuffWeak, 3}, {kActionNone}}};
const Card card_crippling_cloud = {
    "Crippling Cloud", 2, 2, &card_crippling_cloud_plus,
    {.skill = 1, .uncommon = 1, .silent = 1},
    {{kActionDebuff, kBuffPoison, 4}, {kActionDebuff, kBuffWeak, 2}, {kActionNone}}};

const Card card_dash_plus = {
    "Dash+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .uncommon = 1, .silent = 1},
    {{kActionBlock, 13}, {kActionAttack, 13, 1}, {kActionNone}}};
const Card card_dash = {
    "Dash", 2, 2, &card_dash_plus,
    {.attack = 1, .targeted = 1, .uncommon = 1, .silent = 1},
    {{kActionBlock, 10}, {kActionAttack, 10, 1}, {kActionNone}}};

// distraction
// endless agony
// escape plan
// eviscerate
// expertise
// finisher
// flechettes

const Card card_footwork_plus = {
    "Footwork+", 1, 1, nullptr,
    {.power = 1, .uncommon = 1, .silent = 1},
    {{kActionBuff, kBuffDexterity, 3}, {kActionNone}}};
const Card card_footwork = {
    "Footwork", 1, 1, &card_footwork_plus,
    {.power = 1, .uncommon = 1, .silent = 1},
    {{kActionBuff, kBuffDexterity, 2}, {kActionNone}}};

// heel hook
// infinite blades

const Card card_led_sweep_plus = {
    "Leg Sweep+", 2, 2, nullptr,
    {.skill = 1, .targeted = 1, .uncommon = 1, .silent = 1},
    {{kActionDebuff, kBuffWeak, 3}, {kActionBlock, 14}, {kActionNone}}};
const Card card_leg_sweep = {
    "Leg Sweep", 2, 2, &card_led_sweep_plus,
    {.skill = 1, .targeted = 1, .uncommon = 1, .silent = 1},
    {{kActionDebuff, kBuffWeak, 2}, {kActionBlock, 11}, {kActionNone}}};

// masterful stab

const Card card_noxious_fumes_plus = {
    "Noxious Fumes+", 1, 1, nullptr,
    {.power = 1, .uncommon = 1, .silent = 1},
    {{kActionBuff, kBuffNoxiousFumes, 3}, {kActionNone}}};
const Card card_noxious_fumes = {
    "Noxious Fumes", 1, 1, &card_noxious_fumes_plus,
    {.power = 1, .uncommon = 1, .silent = 1},
    {{kActionBuff, kBuffNoxiousFumes, 2}, {kActionNone}}};

// predator
// reflex

const Card card_riddle_with_holes_plus = {
    "Riddle with Holes+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .uncommon = 1, .silent = 1},
    {{kActionAttack, 4, 5}, {kActionNone}}};
const Card card_riddle_with_holes = {
    "Riddle with Holes", 2, 2, &card_riddle_with_holes_plus,
    {.attack = 1, .targeted = 1, .uncommon = 1, .silent = 1},
    {{kActionAttack, 3, 5}, {kActionNone}}};

// setup
// skewer
// tactician

const Card card_terror_plus = {
    "Terror+", 0, 0, nullptr,
    {.skill = 1, .targeted = 1, .exhausts = 1, .uncommon = 1, .silent = 1},
    {{kActionDebuff, kBuffVulnerable, 99}, {kActionNone}}};
const Card card_terror = {
    "Terror", 1, 1, &card_terror_plus,
    {.skill = 1, .targeted = 1, .exhausts = 1, .uncommon = 1, .silent = 1},
    {{kActionDebuff, kBuffVulnerable, 99}, {kActionNone}}};

// well-laid plans

///////////////////////
// rare silent cards //
///////////////////////

// a thousand cuts

const Card card_adrenaline_plus = {
    "Adrenaline+", 0, 0, nullptr,
    {.skill = 1, .exhausts = 1, .rare = 1, .silent = 1},
    {{kActionGainEnergy, 2}, {kActionDrawCards, 2}, {kActionNone}}};
const Card card_adrenaline = {
    "Adrenaline", 0, 0, &card_adrenaline_plus,
    {.skill = 1, .exhausts = 1, .rare = 1, .silent = 1},
    {{kActionGainEnergy, 1}, {kActionDrawCards, 2}, {kActionNone}}};

// after image
// alchemize
// bullet time
// burst
// corpse explosion

const Card card_die_die_die_plus = {
    "Die Die Die+", 1, 1, nullptr,
    {.attack = 1, .exhausts = 1, .rare = 1, .silent = 1},
    {{kActionAttackAll, 17, 1}, {kActionNone}}};
const Card card_die_die_die = {
    "Die Die Die", 1, 1, &card_die_die_die_plus,
    {.attack = 1, .exhausts = 1, .rare = 1, .silent = 1},
    {{kActionAttackAll, 13, 1}, {kActionNone}}};

// doppelganger
// envenom
// glass knife
// grand finale
// malaise
// nightmare
// phantasmal killer
// storm of steel
// tools of the trade
// unload
// wraith form
