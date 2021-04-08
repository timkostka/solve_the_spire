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
    kFightAct1EasyCultist,
    kFightAct1EasyJawWorm,
    kFightAct1EasyLouses,
    kFightAct1EliteLagavulin,
    kFightAct1EliteGremlinNob,
    kFightTestOneLouse,
};

// fight generation
typedef std::vector<std::pair<double, std::vector<Monster>>>(*MonsterGenerationFunction)();

// 
struct FightStruct {
    // name of the fight
    std::string name;
    // monster generation function
    MonsterGenerationFunction generation_function;
};

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

// generate mobs
std::vector<std::pair<double, std::vector<Monster>>> GenerateFightCultist() {
    return GenerateFightSingleMob(base_mob_cultist);
}

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
    {kFightAct1EasyCultist, {"Cultist", GenerateFightCultist}},
    {kFightAct1EasyLouses, {"Two Louses", GenerateFightTwoLouses}},
    {kFightTestOneLouse, {"One Louse", GenerateFightOneLouse}},
};
