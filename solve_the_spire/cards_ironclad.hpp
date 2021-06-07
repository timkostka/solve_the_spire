#pragma once

#include "cards.hpp"

// starting ironclad cards

const Card card_bash_plus = {
    "Bash+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .starting = 1, .ironclad = 1},
    {{kActionAttack, 10, 1}, {kActionDebuff, kBuffVulnerable, 3}, {kActionNone}}};
const Card card_bash = {
    "Bash", 2, 2, &card_bash_plus,
    {.attack = 1, .targeted = 1, .starting = 1, .ironclad = 1},
    {{kActionAttack, 8, 1}, {kActionDebuff, kBuffVulnerable, 2}, {kActionNone}}};

// common ironclad cards

const Card card_anger_plus = {
    "Anger+", 0, 0, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 8, 1}, {kActionAddCardToDiscardPile, card_anger_plus.GetIndex(), 1}, {kActionNone}}};
const Card card_anger = {
    "Anger", 0, 0, &card_anger_plus,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 6, 1}, {kActionAddCardToDiscardPile, card_anger.GetIndex(), 1}, {kActionNone}}};

// armaments

const Card card_body_slam_plus = {
    "Body Slam+", 0, 0, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttackBodySlam}, {kActionNone}}};
const Card card_body_slam = {
    "Body Slam", 1, 1, & card_body_slam_plus,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttackBodySlam}, {kActionNone}}};

// clash

const Card card_cleave_plus = {
    "Cleave+", 1, 1, nullptr,
    {.attack = 1, .common = 1, .ironclad = 1},
    {{kActionAttackAll, 11, 1}, {kActionNone}}};
const Card card_cleave = {
    "Cleave", 1, 1, &card_cleave_plus,
    {.attack = 1, .common = 1, .ironclad = 1},
    {{kActionAttackAll, 8, 1}, {kActionNone}}};

const Card card_clothesline_plus = {
    "Clothesline+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 14, 1}, {kActionDebuff, kBuffWeak, 3}, {kActionNone}}};
const Card card_clothesline = {
    "Clothesline", 2, 2, &card_clothesline_plus,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 12, 1}, {kActionDebuff, kBuffWeak, 2}, {kActionNone}}};

const Card card_flex_plus = {
    "Flex+", 1, 1, nullptr,
    {.skill = 1, .common = 1, .ironclad = 1},
    {{kActionBuff, kBuffStrength, 4}, {kActionBuff, kBuffStrengthDown, 4}, {kActionNone}}};
const Card card_flex = {
    "Flex", 1, 1, &card_flex_plus,
    {.skill = 1, .common = 1, .ironclad = 1},
    {{kActionBuff, kBuffStrength, 2}, {kActionBuff, kBuffStrengthDown, 2}, {kActionNone}}};

// havoc

// headbutt

const Card card_heavy_blade_plus = {
    "Heavy Blade+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttackHeavyBlade, 14, 5}, {kActionNone}}};
const Card card_heavy_blade = {
    "Heavy Blade", 2, 2, &card_heavy_blade_plus,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttackHeavyBlade, 14, 3}, {kActionNone}}};

const Card card_iron_wave_plus = {
    "Iron Wave+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 7, 1}, {kActionBlock, 7}, {kActionNone}}};
const Card card_iron_wave = {
    "Iron Wave", 1, 1, &card_iron_wave_plus,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 5, 1}, {kActionBlock, 5}, {kActionNone}}};

const Card card_perfected_strike_plus = {
    "Perfected Strike+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttackPerfectedStrike, 9, 3}, {kActionNone}}};
const Card card_perfected_strike = {
    "Perfected Strike", 2, 2, &card_perfected_strike_plus,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttackPerfectedStrike, 8, 2}, {kActionNone}}};

const Card card_pommel_strike_plus = {
    "Pommel Strike+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 10, 1}, {kActionDrawCards, 2}, {kActionNone}}};
const Card card_pommel_strike = {
    "Pommel Strike", 1, 1, & card_pommel_strike_plus,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 9, 1}, {kActionDrawCards, 1}, {kActionNone}}};

const Card card_shrug_it_off_plus = {
    "Shrug It Off+", 1, 1, nullptr,
    {.skill = 1, .common = 1, .ironclad = 1},
    {{kActionBlock, 11}, {kActionDrawCards, 1}, {kActionNone}}};
const Card card_shrug_it_off = {
    "Shrug It Off", 1, 1, & card_shrug_it_off_plus,
    {.skill = 1, .common = 1, .ironclad = 1},
    {{kActionBlock, 8}, {kActionDrawCards, 1}, {kActionNone}}};

const Card card_sword_boomerang_plus = {
    "Sword Boomerang+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 3, 4}, {kActionNone}}};
const Card card_sword_boomerang = {
    "Sword Boomerang", 1, 1, &card_sword_boomerang_plus,
    {.attack = 1, .targeted = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 3, 3}, {kActionNone}}};

const Card card_thunderclap_plus = {
    "Thunderclap+", 1, 1, nullptr,
    {.attack = 1, .common = 1, .ironclad = 1},
    {{kActionAttackAll, 7, 1}, {kActionDebuffAll, kBuffVulnerable, 1}, {kActionNone}}};
const Card card_thunderclap = {
    "Thunderclap", 1, 1, &card_thunderclap_plus,
    {.attack = 1, .common = 1, .ironclad = 1},
    {{kActionAttackAll, 4, 1}, {kActionDebuffAll, kBuffVulnerable, 1}, {kActionNone}}};

// true grit

const Card card_twin_strike_plus = {
    "Twin Strike+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 7, 2}, {kActionNone}}};
const Card card_twin_strike = {
    "Twin Strike", 1, 1, &card_twin_strike_plus,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 5, 2}, {kActionNone}}};

// warcry

const Card card_wild_strike_plus = {
    "Wild Strike+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 17, 1}, {kActionAddCardToDrawPile, card_wound.GetIndex(), 1}, {kActionNone}}};
const Card card_wild_strike = {
    "Wild Strike", 1, 1, &card_wild_strike_plus,
    {.attack = 1, .targeted = 1, .strike = 1, .common = 1, .ironclad = 1},
    {{kActionAttack, 12, 1}, {kActionAddCardToDrawPile, card_wound.GetIndex(), 1}, {kActionNone}}};

// uncommon ironclad cards

// battle trance

// blood for blood

// bloodletting

// burning pact

const Card card_carnage_plus = {
    "Carnage+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .ethereal = 1, .uncommon = 1, .ironclad = 1},
    {{kActionAttack, 28, 1}, {kActionNone}}};
const Card card_carnage = {
    "Carnage", 2, 2, &card_carnage_plus,
    {.attack = 1, .targeted = 1, .ethereal = 1, .uncommon = 1, .ironclad = 1},
    {{kActionAttack, 20, 1}, {kActionNone}}};

// combust

// dark embrace

const Card card_disarm_plus = {
    "Disarm+", 1, 1, nullptr,
    {.skill = 1, .exhausts = 1, .uncommon = 1, .ironclad = 1},
    {{kActionDebuff, kBuffStrength, -3}, {kActionNone}}};
const Card card_disarm = {
    "Disarm", 1, 1, & card_disarm_plus,
    {.skill = 1, .exhausts = 1, .uncommon = 1, .ironclad = 1},
    {{kActionDebuff, kBuffStrength, -2}, {kActionNone}}};

// dropkick

// dual wield

// entrench

// evolve

// feel no pain

// fire breathing

// flame barrier

const Card card_ghostly_armor_plus = {
    "Ghostly Armor+", 1, 1, nullptr,
    {.skill = 1, .ethereal = 1, .uncommon = 1, .ironclad = 1},
    {{kActionBlock, 13}, {kActionNone}}};
const Card card_ghostly_armor = {
    "Ghostly Armor", 1, 1, & card_ghostly_armor_plus,
    {.skill = 1, .ethereal = 1, .uncommon = 1, .ironclad = 1},
    {{kActionBlock, 10}, {kActionNone}}};

const Card card_hemokinesis_plus = {
    "Hemokinesis+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .uncommon = 1, .ironclad = 1},
    {{kActionLoseHP, 2}, {kActionAttack, 20, 1}, {kActionNone}}};
const Card card_hemokinesis = {
    "Hemokinesis", 1, 1, & card_hemokinesis_plus,
    {.attack = 1, .targeted = 1, .uncommon = 1, .ironclad = 1},
    {{kActionLoseHP, 2}, {kActionAttack, 15, 1}, {kActionNone}}};

// infernal blade

const Card card_inflame_plus = {
    "Inflame+", 1, 1, nullptr,
    {.skill = 1, .uncommon = 1, .ironclad = 1},
    {{kActionBuff, kBuffStrength, 3}, {kActionNone}}};
const Card card_inflame = {
    "Inflame", 1, 1, &card_inflame_plus,
    {.skill = 1, .uncommon = 1, .ironclad = 1},
    {{kActionBuff, kBuffStrength, 2}, {kActionNone}}};

const Card card_intimidate_plus = {
    "Intimidate+", 0, 0, nullptr,
    {.skill = 1, .uncommon = 1, .ironclad = 1},
    {{kActionDebuffAll, kBuffWeak, 2}, {kActionNone}}};
const Card card_intimidate = {
    "Intimidate", 0, 0, & card_intimidate_plus,
    {.skill = 1, .uncommon = 1, .ironclad = 1},
    {{kActionDebuffAll, kBuffWeak, 1}, {kActionNone}}};

const Card card_metallicize_plus = {
    "Metallicize+", 1, 1, nullptr,
    {.skill = 1, .uncommon = 1, .ironclad = 1},
    {{kActionBuff, kBuffMetallicize, 4}, {kActionNone}}};
const Card card_metallicize = {
    "Metallicize", 1, 1, & card_metallicize_plus,
    {.skill = 1, .uncommon = 1, .ironclad = 1},
    {{kActionBuff, kBuffMetallicize, 3}, {kActionNone}}};

const Card card_power_through_plus = {
    "Power Through+", 1, 1, nullptr,
    {.skill = 1, .uncommon = 1, .ironclad = 1},
    {{kActionBlock, 15}, {kActionAddCardToHand, card_wound.GetIndex(), 2}, {kActionNone}}};
const Card card_power_through = {
    "Power Through", 1, 1, & card_power_through_plus,
    {.skill = 1, .uncommon = 1, .ironclad = 1},
    {{kActionBlock, 15}, {kActionAddCardToHand, card_wound.GetIndex(), 2}, {kActionNone}}};

const Card card_pummel_plus = {
    "Pummel+", 1, 1, nullptr,
    {.attack = 1, .targeted = 1, .exhausts = 1, .uncommon = 1, .ironclad = 1},
    {{kActionAttack, 2, 5}, {kActionNone}}};
const Card card_pummel = {
    "Pummel", 1, 1, & card_pummel_plus,
    {.attack = 1, .targeted = 1, .exhausts = 1, .uncommon = 1, .ironclad = 1},
    {{kActionAttack, 2, 4}, {kActionNone}}};

const Card card_rage_plus = {
    "Rage+", 1, 1, nullptr,
    {.skill = 1, .uncommon = 1, .ironclad = 1},
    {{kActionBuff, kBuffRage, 5}, {kActionNone}}};
const Card card_rage = {
    "Rage", 1, 1, & card_rage_plus,
    {.skill = 1, .uncommon = 1, .ironclad = 1},
    {{kActionBuff, kBuffRage, 3}, {kActionNone}}};

// rampage

const Card card_reckless_charge_plus = {
    "Reckless Charge+", 0, 0, nullptr,
    {.attack = 1, .targeted = 1, .uncommon = 1, .ironclad = 1},
    {{kActionAttack, 10, 1}, {kActionAddCardToDrawPile, card_dazed.GetIndex(), 1}, {kActionNone}}};
const Card card_reckless_charge = {
    "Reckless Charge", 0, 0, & card_reckless_charge_plus,
    {.attack = 1, .targeted = 1, .uncommon = 1, .ironclad = 1},
    {{kActionAttack, 10, 1}, {kActionAddCardToDrawPile, card_dazed.GetIndex(), 1}, {kActionNone}}};

// rupture

// searing blow

// second wind

const Card card_seeing_red_plus = {
    "Seeing Red+", 0, 0, nullptr,
    {.skill = 1, .exhausts = 1, .uncommon = 1, .ironclad = 1},
    {{kActionGainEnergy, 2}, {kActionNone}}};
const Card card_seeing_red = {
    "Seeing Red", 1, 1, & card_seeing_red_plus,
    {.skill = 1, .exhausts = 1, .uncommon = 1, .ironclad = 1},
    {{kActionGainEnergy, 2}, {kActionNone}}};

// sentinel

// sever soul

const Card card_shockwave_plus = {
    "Shockwave+", 2, 2, nullptr,
    {.skill = 1, .targeted = 1, .exhausts = 1, .uncommon = 1, .ironclad = 1},
    {{kActionDebuffAll, kBuffWeak, 5}, {kActionDebuffAll, kBuffVulnerable, 5}, {kActionNone}}};
const Card card_shockwave = {
    "Shockwave", 2, 2, & card_shockwave_plus,
    {.skill = 1, .targeted = 1, .exhausts = 1, .uncommon = 1, .ironclad = 1},
    {{kActionDebuffAll, kBuffWeak, 3}, {kActionDebuffAll, kBuffVulnerable, 3}, {kActionNone}}};

// spot weakness

const Card card_uppercut_plus = {
    "Uppercut+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .uncommon = 1, .ironclad = 1},
    {{kActionAttack, 13, 1}, {kActionDebuff, kBuffWeak, 2}, {kActionDebuff, kBuffVulnerable, 2}, {kActionNone}}};
const Card card_uppercut = {
    "Uppercut", 2, 2, & card_uppercut_plus,
    {.attack = 1, .targeted = 1, .uncommon = 1, .ironclad = 1},
    {{kActionAttack, 13, 1}, {kActionDebuff, kBuffWeak, 1}, {kActionDebuff, kBuffVulnerable, 1}, {kActionNone}}};

const Card card_whirlwind_plus = {
    "Whirlwind+", 0, 0, nullptr,
    {.attack = 1, .x_cost = 1, .uncommon = 1, .ironclad = 1},
    {{kActionAttackAllWhirlwind, 8}, {kActionNone}}};
const Card card_whirlwind = {
    "Whirlwind", 0, 0, & card_whirlwind_plus,
    {.attack = 1, .x_cost = 1, .uncommon = 1, .ironclad = 1},
    {{kActionAttackAllWhirlwind, 5}, {kActionNone}}};

// rare ironclad cards

// barricade

// berserk

const Card card_bludgeon_plus = {
    "Bludgeon+", 3, 3, nullptr,
    {.attack = 1, .targeted = 1, .rare = 1, .ironclad = 1},
    {{kActionAttack, 42, 1}, {kActionNone}}};
const Card card_bludgeon = {
    "Bludgeon", 3, 3, & card_bludgeon_plus,
    {.attack = 1, .targeted = 1, .rare = 1, .ironclad = 1},
    {{kActionAttack, 32, 1}, {kActionNone}}};

// brutality

// corruption

// demon form

// double tap

// exhume

// feed

// fiend fire

const Card card_immolate_plus = {
    "Immolate+", 2, 2, nullptr,
    {.attack = 1, .targeted = 1, .rare = 1, .ironclad = 1},
    {{kActionAttack, 28, 1}, {kActionAddCardToDiscardPile, card_burn.GetIndex(), 1}, {kActionNone}}};
const Card card_immolate = {
    "Immolate", 2, 2, & card_immolate_plus,
    {.attack = 1, .targeted = 1, .rare = 1, .ironclad = 1},
    {{kActionAttack, 21, 1}, {kActionAddCardToDiscardPile, card_burn.GetIndex(), 1}, {kActionNone}}};

const Card card_impervious_plus = {
    "Impervious+", 2, 2, nullptr,
    {.skill = 1, .exhausts = 1, .rare = 1, .ironclad = 1},
    {{kActionBlock, 40}, {kActionNone}}};
const Card card_impervious = {
    "Impervious", 2, 2, & card_impervious_plus,
    {.skill = 1, .exhausts = 1, .rare = 1, .ironclad = 1},
    {{kActionBlock, 30}, {kActionNone}}};

// juggernaut

// limit break

// offering

// reaper
