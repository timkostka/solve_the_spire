#pragma once

#include "cards.hpp"

// uncommon colorless cards
// (there are no common colorless cards)

const Card card_bandage_up = {
    "Bandage Up", 0, 0, nullptr,
    {.skill = 1, .exhausts = 1, .uncommon = 1, .colorless = 1},
    {{kActionHeal, 4}, {kActionNone}}};

const Card card_blind = {
    "Blind", 0, 0, nullptr,
    {.skill = 1, .uncommon = 1, .colorless = 1},
    {{kActionDebuff, kBuffWeak, 2}, {kActionNone}}};

const Card card_dark_shackles = {
    "Dark Shackles", 0, 0, nullptr,
    {.skill = 1, .uncommon = 1, .colorless = 1},
    {{kActionDebuff, kBuffStrength, -9}, {kActionDebuff, kBuffStrengthDown, -9}, {kActionNone}}};

// deep breath

// discovery

// dramatic entrance

// enlightenment

// finesse

const Card card_flash_of_steel = {
    "Flash of Steel", 0, 0, nullptr,
    {.attack = 1, .uncommon = 1, .colorless = 1},
    {{kActionAttack, 3, 1}, {kActionDrawCards, 1}}};

// forethought

// good instincts

// impatience

// jack of all trades

// madness

// mind blast

// panacea

// panic button

// purity

// swift strike

// trip

// rare colorless cards

// Apotheosis

// Chrysalis

// Hand of Greed

// Magnetism

// Master of Strategy

// Mayhem

// Metamorphosis

// Panache

// Sadistic Nature

// Secret Technique

// Secret Weapon

// The Bomb

// Thinking Ahead

// Transmutation

// Violence

// special colorless cards
