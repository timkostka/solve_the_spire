#pragma once

#include <cstdint>

// stances
enum StanceEnum : uint8_t {
    kStanceNone,
    kStanceWrath,
    kStanceCalm,
};

// action type
enum ActionType : uint8_t {
    // no action
    kActionNone = 0,
    // generate the battle
    kActionGenerateBattle,
    // generate mob intents
    kActionGenerateMobIntents,
    // attack a single enemy
    // first argument is attack amount
    kActionAttack,
    // attack a single enemy one time
    // first argument is base amount
    // second argument is additional for each "strike" card
    kActionAttackPerfectedStrike,
    // attack a single enemy one time
    // first argument is base amount
    // second argument is additional amount for each strength
    kActionAttackHeavyBlade,
    // attack a single enemy
    // first argument is amount per enemy in combat
    kActionAttackBowlingBash,
    // attack all enemies
    // first argument is attack amount
    // second argumetn is number of attacks
    kActionAttackAll,
    // attack all enemies for the number of energy we have times
    // first argument is attack amount
    kActionAttackAllWhirlwind,
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
    // lose HP
    // first argument is amount to lose
    kActionLoseHP,
    // add a card to the draw pile
    // first argument is card index
    // second argument is number of cards to add
    kActionAddCardToDrawPile,
    // add a card to the exhaust pile
    // first argument is card index
    // second argument is number of cards to add
    kActionAddCardToDiscardPile,
    // add a card to your hand
    // first argument is card index
    // second argument is number of cards to add
    kActionAddCardToHand,
    // draw cards
    // first argument is how many cards to draw
    kActionDrawCards,
    // change stance
    // first argument is new stance
    kActionChangeStance,
    // gain energy
    // first argument is amount of energy to gain
    kActionGainEnergy,
    // channel one or more orbs
    // first argument is orb type
    // second argument is orb number
    kActionChannelOrb,
    // evoke an orb
    // first argument is number of times to evoke
    kActionEvokeOrb,
    // if the last card played was a skill, do the next action (else skip it)
    kActionLastCardSkill,
    // if the last card played was an attack, do the next action (else skip it)
    kActionLastCardAttack,
    // scry
    // first argument is amount to scry
    kActionScry,
    // gain mantra
    // first argument is amount to gain
    kActionGainMantra,
    // if in wrath stance, do the next action (else skip it)
    kActionInWrath,
    // heal
    // first argument is amount to heal
    kActionHeal,
    // attack for the current amount of block
    kActionAttackBodySlam,
    // upgrade one card in hand
    kUpgradeOneCardInHand,
    // upgrade call cards in hand
    kUpgradeAllCardsInHand,
};

// An action is a single event caused by a card such as damage, or a buff
struct Action {
    // action type
    ActionType type;
    // arguments (meaning depends on action type)
    int16_t arg[2];
};
