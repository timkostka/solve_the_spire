#pragma once

#include <utility>
#include <cstdint>
#include <string>
#include <vector>

#include "action.hpp"
#include "buff_state.hpp"

// list of intent possibilities
typedef std::vector<std::pair<double, uint8_t>> IntentPossibilites;

// typedef of an intent function
struct Monster;
typedef IntentPossibilites (*IntentFunction)(Monster &);

// enum for fight type
enum FightEnum : uint8_t {
    kFightAct1EasyCultist,
    kFightAct1EasyJawWorm,
    kFightAct1EliteLagavulin,
    kFightAct1EliteGremlinNob,
};

// monster intent and actions
struct MonsterIntent {
    // name of intent
    std::string name;
    // actions
    Action action[2];
};

// a BaseMonster holds information on how to generate a new monster
struct BaseMonster {
    // name
    std::string name;
    // monster hp range
    std::pair<uint16_t, uint16_t> hp_range;
    // list of intents
    MonsterIntent intent[3];
    // intent function
    IntentFunction intent_function;
};

// a Monster holds state information
struct Monster {
    // pointer to base monster, or nullptr for no monster
    const BaseMonster * base;
    // hp
    uint16_t hp;
    // max hp
    uint16_t max_hp;
    // turn
    //uint8_t turn;
    // last 2 intents (initialized to 255)
    uint8_t last_intent[3];
    // block amount
    uint8_t block;
    // buffs
    BuffState buff;
    // default constructor (no mob)
    Monster() : base(nullptr), hp(0), last_intent{0}, block(0) {
    }
    // construct from base monster
    Monster(BaseMonster & base_) {
        base = &base_;
        hp = base->hp_range.first;
        max_hp = hp;
        last_intent[0] = 255;
        block = 0;
        buff = BuffState();
    }
    // block for X amount (before modifiers)
    void Block(int16_t amount) {
        amount += buff[kBuffDexterity];
        if (amount > 0) {
            block += amount;
        }
    }
    // return true if this mob exists
    bool Exists() const {
        return base != nullptr && hp > 0;
    }
    // get attacked for X damage
    void Attack(uint16_t damage) {
        // apply vulnerability
        if (buff.value[kBuffVulnerable]) {
            damage = (uint16_t) (damage * 1.5);
        }
        // reduce block
        if (block) {
            if (block >= damage) {
                block -= damage;
                return;
            } else {
                damage -= block;
                block = 0;
            }
        }
        // reduce HP
        if (hp <= damage) {
            hp = 0;
        } else {
            hp -= damage;
        }
    }
    // return true if monster is dead
    bool IsDead() const {
        return hp == 0;
    }
    // find a new intent
    // returned as a vector of (probability, intent index)
    std::vector<std::pair<double, uint8_t>> GetIntents() {
        return base->intent_function(*this);
    }
    // select a new intent
    void SelectIntent(uint8_t intent_index) {
        last_intent[2] = last_intent[1];
        last_intent[1] = last_intent[0];
        last_intent[0] = intent_index;
    }
};

// get intent of Cultist
IntentPossibilites GetIntentCultist(Monster & mob) {
    IntentPossibilites result;
    if (mob.last_intent[0] == 255) {
        result.push_back(std::pair<double, uint8_t>(1.0, 0));
    } else {
        result.push_back(std::pair<double, uint8_t>(1.0, 1));
    }
    return result;
}

// base models for each mob are below
BaseMonster base_mob_cultist = {
    "Cultist",
    {50, 56},
    {
        {"Incantation", {{kActionBuff, kBuffRitual, 5}}},
        {"Dark Strike", {{kActionAttack, 6}}},
    },
    GetIntentCultist,
};

// get intent of Jaw Worm
IntentPossibilites GetIntentJawWorm(Monster & mob) {
    IntentPossibilites result;
    double x = rand() / RAND_MAX;
    if (mob.last_intent[0] == 0) {
        // cannot chomp twice in a row
        result.push_back(std::pair<double, uint8_t>(0.30 / 0.55, 1));
        result.push_back(std::pair<double, uint8_t>(0.25 / 0.55, 2));
    } else if (mob.last_intent[0] == 2) {
        // cannot bellow twice in a row
        result.push_back(std::pair<double, uint8_t>(0.45 / 0.75, 0));
        result.push_back(std::pair<double, uint8_t>(0.30 / 0.75, 1));
    } else if (mob.last_intent[0] == 1 && mob.last_intent[1] == 1) {
        // cannot thrash 3x in a row
        result.push_back(std::pair<double, uint8_t>(0.45 / 0.7, 0));
        result.push_back(std::pair<double, uint8_t>(0.25 / 0.7, 2));
    } else {
        // can do any attack
        result.push_back(std::pair<double, uint8_t>(0.45, 0));
        result.push_back(std::pair<double, uint8_t>(0.30, 1));
        result.push_back(std::pair<double, uint8_t>(0.25, 2));
    }
    return result;
}

// base models for each mob are below
BaseMonster base_mob_jaw_worm = {
    "Jaw Worm",
    {42, 46},
    {
        {"Chomp", {{kActionAttack, 12}}},
        {"Thrash", {{kActionAttack, 7}, {kActionBlock, 5}}},
        {"Bellow", {{kActionBuff, kBuffStrength, 5}, {kActionBlock, 9}}},
    },
    GetIntentJawWorm,
};

// get intent of Lagavulin
IntentPossibilites GetIntentLagavulin(Monster & mob) {
    IntentPossibilites result;
    // sleep first
    if (mob.last_intent[0] == 255) {
        mob.block = 8;
        mob.buff[kBuffMetallicize] = 8;
        result.push_back(std::pair<double, uint8_t>(1.0, 0));
    } else if (mob.hp == mob.max_hp && mob.last_intent[0] == 0) {
        result.push_back(std::pair<double, uint8_t>(1.0, 0));
    } else if (mob.last_intent[0] == 1 && mob.last_intent[1] == 1) {
        result.push_back(std::pair<double, uint8_t>(1.0, 2));
    } else {
        mob.buff[kBuffMetallicize] = 0;
        result.push_back(std::pair<double, uint8_t>(1.0, 1));
    }
    return result;
}

// base models for each mob are below
BaseMonster base_mob_lagavulin = {
    "Lagavulin",
    {70, 70}, //{112, 115},
    {
        {"Sleep", {{kActionNone}}},
        {"Attack", {{kActionAttack, 20}}},
        {"Siphon Soul", {{kActionDebuff, kBuffStrength, -2},
                         {kActionDebuff, kBuffDexterity, -2}}},
    },
    GetIntentLagavulin,
};

// get intent of Gremlin Nob
IntentPossibilites GetIntentGremlinNob(Monster & mob) {
    IntentPossibilites result;
    // sleep first
    if (mob.last_intent[0] == 255) {
        result.push_back(std::pair<double, uint8_t>(1.0, 0));
    } else if (mob.last_intent[0] != 2 && mob.last_intent[1] != 2) {
        result.push_back(std::pair<double, uint8_t>(1.0, 2));
    } else {
        result.push_back(std::pair<double, uint8_t>(1.0, 1));
    }
    return result;
}

// base models for each mob are below
BaseMonster base_mob_gremlin_nob = {
    "Gremlin Nob",
    {85, 85}, //{85, 90},
    {
        {"Bellow", {{kActionBuff, kBuffEnrage, 3}}},
        {"Rush", {{kActionAttack, 16}}},
        {"Skull Bash", {{kActionAttack, 8},
                         {kActionDebuff, kBuffVulnerable, 2}}},
    },
    GetIntentGremlinNob,
};
