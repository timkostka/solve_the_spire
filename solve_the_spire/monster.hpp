#pragma once

#include <utility>
#include <cstdint>
#include <string>
#include <vector>

#include "defines.h"
#include "action.hpp"
#include "buff_state.hpp"

// list of intent possibilities
typedef std::vector<std::pair<double, uint8_t>> IntentPossibilites;

// typedef of an intent function
struct Monster;
typedef IntentPossibilites (*IntentFunction)(Monster &);

// monster intent and actions
struct MonsterIntent {
    // name of intent
    std::string name;
    // actions
    Action action[2];
};

// base monster flags
enum MonsterFlag : uint8_t {
    kMonsterFlagElite = 1 << 0,
    kMonsterFlagBoss = 1 << 1,
    kMonsterFlagMinion = 1 << 2,
    kMonsterFlagTest = 1 << 3,
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
    // monster flags
    MonsterFlag flag;
};

// a Monster holds state information
struct Monster {
    // pointer to base monster, or nullptr for no monster
    const BaseMonster * base;
    // hp
    uint16_t hp;
    // max hp
    uint16_t max_hp;
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
    Monster(const BaseMonster & base_) {
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
    // take X direct HP damage
    void TakeHPLoss(uint16_t damage, bool attack_damage) {
        assert(damage > 0);
        // reduce HP
        if (hp <= damage) {
            hp = 0;
        } else {
            hp -= damage;
            if (attack_damage && hp > 0 && buff[kBuffCurlUp]) {
                block += buff[kBuffCurlUp];
                buff[kBuffCurlUp] = 0;
            }
        }
    }
    // take X damage
    void TakeDamage(uint16_t damage, bool attack_damage) {
        // block what we can
        if (block) {
            if (block >= damage) {
                block -= damage;
                damage = 0;
                return;
            } else {
                damage -= block;
                block = 0;
            }
        }
        if (damage) {
            TakeHPLoss(damage, attack_damage);
        }
    }
    // get attacked for X damage
    void Attack(uint16_t damage) {
        // apply vulnerability
        if (buff.value[kBuffVulnerable]) {
            damage = (uint16_t) (damage * 1.5);
        }
        TakeDamage(damage, true);
    }
    // return true if monster is dead
    bool IsDead() const {
        return hp == 0;
    }
    // return true if mob is an elite
    bool IsElite() const {
        return base->flag & kMonsterFlagElite;
    }
    // return true if mob is an elite
    bool IsBoss() const {
        return base->flag & kMonsterFlagBoss;
    }
    // return true if mob is an elite
    bool IsMinion() const {
        return base->flag & kMonsterFlagMinion;
    }
    // returned as a vector of (probability, intent index)
    std::vector<std::pair<double, uint8_t>> GetIntents() {
        if (base->intent_function == nullptr) {
            std::vector<std::pair<double, uint8_t>> result;
            result.push_back(std::pair<double, uint8_t>(1.0, 0));
            return result;
        }
        return base->intent_function(*this);
    }
    // select a new intent
    void SelectIntent(uint8_t intent_index) {
        last_intent[2] = last_intent[1];
        last_intent[1] = last_intent[0];
        last_intent[0] = intent_index;
    }
    // convert to a string
    std::string ToString() const {
        if (!Exists()) {
            return "";
        }
        std::string result = base->name;
        result += "(";
        result += std::to_string(hp) + "/" + std::to_string(max_hp);
        if (last_intent[0] != 255) {
            result += ", " + base->intent[last_intent[0]].name;
        }
        auto buff_string = buff.ToString();
        if (!buff_string.empty()) {
            result += ", " + buff.ToString();
        }
        result += ")";
        return result;
    }
};

// generate possibilities for the given base mob
std::vector<std::pair<double, Monster>> GenerateMob(const BaseMonster & base) {
    std::vector<std::pair<double, Monster>> result;
    // if we're just using an average, return a single mob
    if (normalize_mob_variations) {
        result.push_back(std::pair<double, Monster>(1.0, Monster(base)));
        result.rbegin()->second.hp =
            (base.hp_range.first + base.hp_range.second) / 2;
        return result;
    }
    // else generate all possible HPs with equal probability
    uint16_t count = base.hp_range.second - base.hp_range.first + 1;
    for (int i = 0; i < count; ++i) {
        result.push_back(std::pair<double, Monster>(1.0 / count, Monster(base)));
        result.rbegin()->second.hp = base.hp_range.first + i;
    }
    return result;
}

// return all possibilites for generating a red or green louse
std::vector<std::pair<double, Monster>> GenerateLouse(const BaseMonster & base) {
    // result
    std::vector<std::pair<double, Monster>> result = GenerateMob(base);
    // strength possibilities (effective)
    {
        std::vector<std::pair<double, unsigned int>> strength;
        if (normalize_mob_variations) {
            strength.push_back(std::pair<double, unsigned int>(1.0, 1));
        } else {
            strength.push_back(std::pair<double, unsigned int>(1.0 / 3, 0));
            strength.push_back(std::pair<double, unsigned int>(1.0 / 3, 1));
            strength.push_back(std::pair<double, unsigned int>(1.0 / 3, 2));
        }
        std::vector<std::pair<double, Monster>> old_result = result;
        result.clear();
        for (auto & this_strength : strength) {
            for (std::pair<double, Monster> item : old_result) {
                item.second.buff[kBuffStrength] = this_strength.second;
                item.first *= this_strength.first;
                result.push_back(item);
            }
        }
    }
    // curl up possibilities
    {
        std::vector<std::pair<double, unsigned int>> curl_up;
        if (normalize_mob_variations) {
            curl_up.push_back(std::pair<double, unsigned int>(1.0, 10));
        } else {
            curl_up.push_back(std::pair<double, unsigned int>(0.25, 9));
            curl_up.push_back(std::pair<double, unsigned int>(0.25, 10));
            curl_up.push_back(std::pair<double, unsigned int>(0.25, 11));
            curl_up.push_back(std::pair<double, unsigned int>(0.25, 12));
        }
        std::vector<std::pair<double, Monster>> old_result = result;
        result.clear();
        for (auto & this_curl_up : curl_up) {
            for (std::pair<double, Monster> item : old_result) {
                item.second.buff[kBuffCurlUp] = this_curl_up.second;
                item.first *= this_curl_up.first;
                result.push_back(item);
            }
        }
    }
    return result;
}

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
    double x = (double) rand() / RAND_MAX;
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

// get intent of Red Louse
IntentPossibilites GetIntentRedLouse(Monster & mob) {
    IntentPossibilites result;
    if (mob.last_intent[0] == 0 && mob.last_intent[1] == 0) {
        result.push_back(std::pair<double, uint8_t>(1.0, 1));
    } else if (mob.last_intent[0] == 1) {
        result.push_back(std::pair<double, uint8_t>(1.0, 0));
    } else {
        result.push_back(std::pair<double, uint8_t>(0.75, 0));
        result.push_back(std::pair<double, uint8_t>(0.25, 1));
    }
    return result;
}

// base models for each mob are below
BaseMonster base_mob_red_louse = {
    "Red Louse",
    {11, 16},
    {
        {"Bite", {{kActionAttack, 6}}},
        {"Grow", {{kActionBuff, kBuffStrength, 4}}},
    },
    GetIntentRedLouse,
};

// base models for each mob are below
BaseMonster base_mob_green_louse = {
    "Green Louse",
    {12, 18},
    {
        {"Bite", {{kActionAttack, 6}}},
        {"Spit Web", {{kActionDebuff, kBuffWeak, 2}}},
    },
    GetIntentRedLouse, // same intent function for red and green
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
    {112, 115},
    {
        {"Sleep", {{kActionNone}}},
        {"Attack", {{kActionAttack, 20}}},
        {"Siphon Soul", {{kActionDebuff, kBuffStrength, -2},
                         {kActionDebuff, kBuffDexterity, -2}}},
    },
    GetIntentLagavulin,
    kMonsterFlagElite,
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
    {85, 90},
    {
        {"Bellow", {{kActionBuff, kBuffEnrage, 3}}},
        {"Rush", {{kActionAttack, 16}}},
        {"Skull Bash", {{kActionAttack, 8},
                         {kActionDebuff, kBuffVulnerable, 2}}},
    },
    GetIntentGremlinNob,
    kMonsterFlagElite,
};

// base model for a test mob with 100hp that always attacks for 10
BaseMonster base_mob_test_100hp_10hp_attacker = {
    "Test Mob #1",
    {100, 100},
    {
        {"Attack", {{kActionAttack, 10}}},
    },
    nullptr,
    kMonsterFlagTest,
};
