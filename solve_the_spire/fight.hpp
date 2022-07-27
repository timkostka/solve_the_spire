#pragma once

#include <utility>
#include <cstdint>
#include <string>
#include <vector>

#include "action.hpp"
#include "buff_state.hpp"
#include "monster.hpp"

// enum for fight type
enum FightEnum : uint8_t {
    kFightNone,
    kFightAct1EasyCultist,
    kFightAct1EasyJawWorm,
    kFightAct1EasyLouses,
    kFightAct1BlueSlaver,
    kFightAct1EliteLagavulin,
    kFightAct1EliteGremlinNob,
    kFightAct1BurningEliteGremlinNob,
    kFightAct1EliteGremlinNobNominal,
    kFightTestOneLouse,
};

// fight generation
typedef std::vector<std::pair<double, std::vector<Monster>>>(*MonsterGenerationFunction)();

// 
struct FightStruct {
    // name of the fight
    std::string name;
    // base mob name (or nullptr)
    BaseMonster * base_mob;
    // true if burning elite
    bool burning_elite;
    // custom monster generation function (or nullptr)
    MonsterGenerationFunction generation_function;
};

// apply superelite buffs to existing layouts
std::vector<std::pair<double, std::vector<Monster>>> ApplyBurningEliteBuffs(std::vector<std::pair<double, std::vector<Monster>>> & layout, int act_number) {
    std::vector<std::pair<double, std::vector<Monster>>> result;
    for (auto & this_layout : layout) {
        // strength
        result.push_back(this_layout);
        result.rbegin()->first *= 0.25;
        for (auto & mob : result.rbegin()->second) {
            mob.buff[kBuffStrength] = act_number + 1;
        }
        // max HP
        result.push_back(this_layout);
        result.rbegin()->first *= 0.25;
        for (auto & mob : result.rbegin()->second) {
            mob.hp += mob.hp / 4;
        }
        // metallicize
        result.push_back(this_layout);
        result.rbegin()->first *= 0.25;
        for (auto & mob : result.rbegin()->second) {
            mob.buff[kBuffMetallicize] = 2 * act_number + 2;
        }
        // regenerate
        result.push_back(this_layout);
        result.rbegin()->first *= 0.25;
        for (auto & mob : result.rbegin()->second) {
            mob.buff[kBuffRegenerate] = 2 * act_number + 1;
        }
    }
    return result;
}


// generate mobs
std::vector<std::pair<double, std::vector<Monster>>> GenerateFightSingleMob(
        const BaseMonster & base_mob) {
    std::vector<std::pair<double, std::vector<Monster>>> result;
    auto mobs = GenerateMob(base_mob);
    for (auto & item : mobs) {
        result.push_back(std::pair<double, std::vector<Monster>>());
        result.rbegin()->first = item.first;
        result.rbegin()->second.push_back(item.second);
    }
    return result;
}

//// generate mobs
//std::vector<std::pair<double, std::vector<Monster>>> GenerateFightCultist() {
//    return GenerateFightSingleMob(base_mob_cultist);
//}
//
//// generate mobs
//std::vector<std::pair<double, std::vector<Monster>>> GenerateFightJawWorm() {
//    return GenerateFightSingleMob(base_mob_jaw_worm);
//}
//
//// generate mobs
//std::vector<std::pair<double, std::vector<Monster>>> GenerateFightGremlinNob() {
//    return GenerateFightSingleMob(base_mob_gremlin_nob);
//}
//
//// generate mobs
//std::vector<std::pair<double, std::vector<Monster>>> GenerateFightLagavulin() {
//    return GenerateFightSingleMob(base_mob_lagavulin);
//}

// generate mobs for the two louse fight
std::vector<std::pair<double, std::vector<Monster>>> GenerateFightTwoLouses() {
    // 50% red louse
    auto red_louses = GenerateLouse(base_mob_red_louse);
    // 50% green louse
    auto green_louses = GenerateLouse(base_mob_green_louse);
    std::vector<std::pair<double, std::vector<Monster>>> result;
    // 25% chance of two red louses
    for (unsigned int i = 0; i < red_louses.size(); ++i) {
        auto & mob_one = red_louses[i];
        for (unsigned int j = i; j < red_louses.size(); ++j) {
            auto & mob_two = red_louses[i];
            result.push_back(std::pair<double, std::vector<Monster>>());
            result.rbegin()->first = 0.25 * mob_one.first * mob_two.first;
            if (i != j) {
                result.rbegin()->first *= 2;
            }
            result.rbegin()->second.push_back(mob_one.second);
            result.rbegin()->second.push_back(mob_two.second);
        }
    }
    // 25% chance of two green louses
    for (unsigned int i = 0; i < green_louses.size(); ++i) {
        auto & mob_one = green_louses[i];
        for (unsigned int j = i; j < green_louses.size(); ++j) {
            auto & mob_two = green_louses[i];
            result.push_back(std::pair<double, std::vector<Monster>>());
            result.rbegin()->first = 0.25 * mob_one.first * mob_two.first;
            if (i != j) {
                result.rbegin()->first *= 2;
            }
            result.rbegin()->second.push_back(mob_one.second);
            result.rbegin()->second.push_back(mob_two.second);
        }
    }
    // 50% chance of one red, one green louse
    for (unsigned int i = 0; i < red_louses.size(); ++i) {
        auto & mob_one = red_louses[i];
        for (unsigned int j = 0; j < green_louses.size(); ++j) {
            auto & mob_two = green_louses[i];
            result.push_back(std::pair<double, std::vector<Monster>>());
            result.rbegin()->first = 0.50 * mob_one.first * mob_two.first;
            result.rbegin()->second.push_back(mob_one.second);
            result.rbegin()->second.push_back(mob_two.second);
        }
    }
    return result;
}

// generate mobs for the two louse fight
std::vector<std::pair<double, std::vector<Monster>>> GenerateFightOneLouse() {
    // 50% red louse
    auto red_louses = GenerateLouse(base_mob_red_louse);
    // 50% green louse
    auto green_louses = GenerateLouse(base_mob_green_louse);
    std::vector<std::pair<double, std::vector<Monster>>> result;
    // 50% red louse
    for (auto & mob_one : red_louses) {
        result.push_back(std::pair<double, std::vector<Monster>>());
        result.rbegin()->first = 0.50 * mob_one.first;
        result.rbegin()->second.push_back(mob_one.second);
    }
    for (auto & mob_one : green_louses) {
        result.push_back(std::pair<double, std::vector<Monster>>());
        result.rbegin()->first = 0.50 * mob_one.first;
        result.rbegin()->second.push_back(mob_one.second);
    }
    return result;
}

// map between fights and generator functions
std::map<FightEnum, FightStruct> fight_map = {
    {kFightAct1EasyCultist, {"Cultist", &base_mob_cultist, false, nullptr}},
    {kFightAct1EasyJawWorm, {"Jaw Worm", &base_mob_jaw_worm, false, nullptr}},
    {kFightAct1EasyLouses, {"Two Louses", nullptr, false, GenerateFightTwoLouses}},
    {kFightAct1BlueSlaver, {"Blue Slaver", &base_mob_blue_slaver, false, nullptr}},
    {kFightTestOneLouse, {"One Louse", nullptr, false, GenerateFightOneLouse}},
    {kFightAct1EliteGremlinNob, {"Gremlin Nob", &base_mob_gremlin_nob, false, nullptr}},
    {kFightAct1BurningEliteGremlinNob, {"Gremlin Nob", &base_mob_gremlin_nob, true, nullptr}},
    {kFightAct1EliteLagavulin, {"Lagavulin", &base_mob_lagavulin, false, nullptr}},
};
