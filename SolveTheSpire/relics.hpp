#pragma once

struct RelicStruct {
    // starting relics
    
    // 1 if we have the relic, 0 otherwise
    unsigned int burning_blood : 1;
    unsigned int ring_of_the_snake : 1;
    //unsigned int cracked_core : 1;
    //unsigned int pure_water : 1;

    // common relics

    unsigned int akabeko : 1;
    // set to 1 after first attack
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

    // boss relics
    //unsigned int busted_crown : 1;
    //unsigned int coffee_dripper : 1;
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
    RelicStruct() {
        memset(this, 0, sizeof(*this));
    }

};

static_assert(sizeof(RelicStruct) == 4, "");
