#pragma once

#include <string>
#include <map>

struct RelicStruct {
    // starting relics
    
    // 1 if we have the relic, 0 otherwise
    unsigned int burning_blood : 1;
    unsigned int ring_of_the_snake : 1;
    unsigned int cracked_core : 1;
    unsigned int pure_water : 1;

    // common relics

    unsigned int akabeko : 1;
    // set to 0 after first attack
    unsigned int akabeko_active : 1;
    unsigned int anchor : 1;
    unsigned int ancient_tea_set : 1;
    unsigned int ancient_tea_set_active : 1;
    //unsigned int art_of_war : 1;
    unsigned int bag_of_marbles : 1;
    unsigned int bag_of_preparation : 1;
    unsigned int blood_vial : 1;
    unsigned int bronze_scales : 1;
    //unsigned int centennial_puzzle : 1;
    //unsigned int centennial_puzzle_active : 1;
    //unsigned int happy_flower : 1;
    //// increase at start of each turn, when it hits 3, add energy and reset to 0
    //unsigned int happy_flower_number : 2;
    unsigned int lantern : 1;
    //unsigned int nunchaku : 1;
    unsigned int oddly_smooth_stone : 1;
    unsigned int orichalcum : 1;
    //unsigned int pen_nib : 1;
    //unsigned int pen_nib_number : 4;
    unsigned int preserved_insect : 1;
    //unsigned int the_boot : 1;
    unsigned int vajra : 1;
    
    // uncommon relics

    //unsigned int gremlin_horn : 1;
    //unsigned int horn_cleat : 1;
    //unsigned int kunai : 1;
    //unsigned int kunai_count : 2;
    //unsigned int letter_opener : 1;
    //unsigned int letter_opener_count : 2;
    unsigned int meat_on_the_bone : 1;
    //unsigned int mercury_hourglass : 1;
    //unsigned int ornamental_fan : 1;
    //unsigned int ornamental_fan_count : 2;
    //unsigned int shuriken : 1;
    //unsigned int shuriken_count : 2;
    //unsigned int paper_phrog : 1;
    //unsigned int self_forming_clay : 1;
    //unsigned int meat_on_the_bone : 1;
    //unsigned int meat_on_the_bone : 1;

    // rare elics

    //unsigned int bird_faced_urn : 1;
    unsigned int calipers : 1;
    //unsigned int captains_wheel : 1;
    //unsigned int dead_branch : 1;
    //unsigned int du_vu_doll : 1;
    //unsigned int follilized_helix : 1;
    //unsigned int gambling_chip : 1;
    //unsigned int ginger : 1;
    //unsigned int girya : 1;
    //unsigned int girya_number : 2;
    unsigned int ice_cream : 1;
    //unsigned int incense_burner : 1;
    //unsigned int incense_burner_number : 3;
    //unsigned int lizard_tail : 1;
    //unsigned int lizard_tail_active : 1;
    //unsigned int mango : 1;
    //unsigned int old_coin : 1;
    //unsigned int peace_pipe : 1;
    //unsigned int pocketwatch : 1;
    //unsigned int prayer_wheel : 1;
    //unsigned int shovel : 1;
    //unsigned int stone_calendar : 1;
    //unsigned int thread_and_needle : 1;
    //unsigned int torii : 1;
    //unsigned int tungsten_rod : 1;
    //unsigned int turnip : 1;
    //unsigned int unceasing_top : 1;
    //unsigned int wing_boots : 1;

    // TODO: class specific rare relics

    // TODO: shop relics

    // boss relics
 
    //unsigned int busted_crown : 1;
    unsigned int coffee_dripper : 1;
    unsigned int cursed_key : 1;
    //unsigned int ectoplasm : 1;
    //unsigned int fusion_hammer : 1;
    //unsigned int philosophers_stone : 1;
    //unsigned int runic_dome : 1;
    unsigned int runic_pyramid : 1;
    //unsigned int slavers_collar : 1;
    //unsigned int snecko_eye : 1;
    //unsigned int sozu : 1;
    //unsigned int velvet_choker : 1;

    //unsigned int cursed_key : 1;

    // constructor
    /*RelicStruct() {
        memset(this, 0, sizeof(*this));
    }*/

    // add the second relic to the first relic set
    void operator += (const RelicStruct & that) {
        auto one_ptr = (uint8_t *) this;
        auto two_ptr = (const uint8_t *) &that;
        for (int i = 0; i < sizeof(*this); ++i) {
            *one_ptr++ |= *two_ptr++;
        }
    }

    // return true if this contains the given relic
    bool Contains(const RelicStruct & that) const {
        auto one_ptr = (uint8_t *) this;
        auto two_ptr = (const uint8_t *) &that;
        for (int i = 0; i < sizeof(*this); ++i) {
            if ((*one_ptr & *two_ptr) != *two_ptr) {
                return false;
            }
            ++one_ptr;
            ++two_ptr;
        }
        return true;
    }

    // convert to string form
    std::string ToString() const;
};

static_assert(sizeof(RelicStruct) == 4, "");

// map between strings and relics
std::map<std::string, RelicStruct> relic_map = {
    {"Burning Blood", {.burning_blood = 1}},
    {"Ring of the Snake", {.ring_of_the_snake = 1}},
    {"Cracked Core", {.cracked_core = 1}},
    {"Pure Water", {.pure_water = 1}},

    {"Akabeko", {.akabeko = 1}},
    {"Anchor", {.anchor = 1}},
    {"Ancient Tea Set", {.ancient_tea_set = 1}},
    {"Bag of Marbles", {.bag_of_marbles = 1}},
    {"Bag of Preparation", {.bag_of_preparation = 1}},

    {"Blood  Vial", {.blood_vial = 1}},
    {"Bronze Scales", {.bronze_scales = 1}},
    {"Lantern", {.lantern = 1}},
    {"Oddly Smooth Stone", {.oddly_smooth_stone = 1}},
    {"Orichalcum", {.orichalcum = 1}},
    {"Vajra", {.preserved_insect = 1}},

    {"Meat on the Bone", {.meat_on_the_bone = 1}},
    {"Cursed Key", {.cursed_key = 1}},
    {"Runic Pyramid", {.runic_pyramid = 1}},
    /*{"Akabeko", {.akabeko = 1}},
    {"Akabeko", {.akabeko = 1}},
    {"Akabeko", {.akabeko = 1}},
    {"Akabeko", {.akabeko = 1}},*/
};

std::string RelicStruct::ToString() const {
    bool first = true;
    std::string result;
    for (auto & item : relic_map) {
        if (Contains(item.second)) {
            if (!first) {
                result += ", ";
            }
            first = false;
            result += item.first;
        }
    }
    return result;
};

//struct test_struct {
//    int x;
//    int y;
//};
//
//test_struct xasdf = {.x = 6, .y = 7};
