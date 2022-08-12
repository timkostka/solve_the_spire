#include <iostream>
#include <list>
#include <vector>
#include <deque>
#include <utility>
#include <ctime>
#include <sstream>
#include <fstream>

#include "defines.h"
#include "presets.hpp"
#include "card_collection.hpp"
#include "cards.hpp"
#include "node.hpp"
#include "fight.hpp"
#include "stopwatch.hpp"
#include "tree.hpp"

/*

--character=ironclad --relics=pure_water --fight=gremlin_nob --hp=full

--character=silent
--deck=5xstrike,5xdefend
--hp=67/100
--relics=ring_of_the_snake
--cards=5xstrike

--relics=none

*/

// get list of relics to compare

// compare relics
void CompareRelics(const TreeStruct & tree, const Node & top_node) {
    std::vector<std::pair<std::string, RelicStruct>> relic_list;
    // add base case
    relic_list.push_back(std::pair<std::string, RelicStruct>("base", RelicStruct()));
    relic_list.rbegin()->second.burning_blood = 1;

    // add differential cases
    relic_list.push_back(*relic_list.begin());
    relic_list.rbegin()->first = "Akabeko";
    relic_list.rbegin()->second.akabeko = 1;
    relic_list.push_back(*relic_list.begin());
    relic_list.rbegin()->first = "Anchor";
    relic_list.rbegin()->second.anchor = 1;
    relic_list.push_back(*relic_list.begin());
    relic_list.rbegin()->first = "bag_of_marbles";
    relic_list.rbegin()->second.bag_of_marbles = 1;
    relic_list.push_back(*relic_list.begin());
    relic_list.rbegin()->first = "bag_of_preparation";
    relic_list.rbegin()->second.bag_of_preparation = 1;
    relic_list.push_back(*relic_list.begin());
    relic_list.rbegin()->first = "blood_vial";
    relic_list.rbegin()->second.blood_vial = 1;
    relic_list.push_back(*relic_list.begin());
    relic_list.rbegin()->first = "bronze_scales";
    relic_list.rbegin()->second.bronze_scales = 1;
    relic_list.push_back(*relic_list.begin());
    relic_list.rbegin()->first = "lantern";
    relic_list.rbegin()->second.lantern = 1;
    relic_list.push_back(*relic_list.begin());
    relic_list.rbegin()->first = "oddly_smooth_stone";
    relic_list.rbegin()->second.oddly_smooth_stone = 1;
    relic_list.push_back(*relic_list.begin());
    relic_list.rbegin()->first = "orichalcum";
    relic_list.rbegin()->second.orichalcum = 1;
    relic_list.push_back(*relic_list.begin());
    relic_list.rbegin()->first = "preserved_insect";
    relic_list.rbegin()->second.preserved_insect = 1;
    relic_list.push_back(*relic_list.begin());
    relic_list.rbegin()->first = "vajra";
    relic_list.rbegin()->second.vajra = 1;
    relic_list.push_back(*relic_list.begin());
    relic_list.rbegin()->first = "cursed_key";
    relic_list.rbegin()->second.cursed_key = 1;

    std::ofstream outFile("relic_comparison.txt");
    std::streambuf * oldCoutStreamBuf = std::cout.rdbuf();
    bool base = true;
    double base_objective = 0;
    outFile << top_node.ToString() << std::endl;
    for (auto & this_list : relic_list) {
        Node this_top_node = top_node;
        this_top_node.relics = this_list.second;
        TreeStruct this_tree(this_top_node);
        this_tree.fight_type = tree.fight_type;
        this_tree.Expand();
        if (base) {
            outFile << this_list.first << ": " << this_top_node.objective << std::endl;
            base_objective = this_top_node.objective;
            base = false;
        } else {
            outFile << this_list.first << ": " << this_top_node.objective;
            outFile << " (";
            double delta = this_top_node.objective - base_objective;
            if (delta > 0) {
                outFile << "+";
            }
            outFile << delta << ")";
            outFile << std::endl;
        }
    }
    outFile.close();


}

// compare the effect on addings cards on the given fight
void CompareIroncladCards(const Node & top_node) {
    std::vector<const Card *> card_list;
    // add base case
    card_list.push_back(nullptr);

    // add cards
    card_list.push_back(&card_anger);
    card_list.push_back(&card_cleave);
    card_list.push_back(&card_clothesline);
    card_list.push_back(&card_flex);
    card_list.push_back(&card_iron_wave);
    card_list.push_back(&card_perfected_strike);
    card_list.push_back(&card_sword_boomerang);
    card_list.push_back(&card_thunderclap);
    card_list.push_back(&card_twin_strike);
    card_list.push_back(&card_wild_strike);

    std::ofstream outFile("card_comparison.txt");
    std::streambuf * oldCoutStreamBuf = std::cout.rdbuf();
    bool base = true;
    double base_objective = 0;
    outFile << top_node.ToString() << std::endl;
    for (auto & this_card_ptr : card_list) {
        Node this_top_node = top_node;
        if (this_card_ptr != nullptr) {
            this_top_node.deck.AddCard(*this_card_ptr);
        }
        this_top_node.InitializeStartingNode();
        TreeStruct tree(this_top_node);
        tree.Expand();
        if (this_card_ptr == nullptr) {
            outFile << "base: " << this_top_node.objective << std::endl;
            base_objective = this_top_node.objective;
            base = false;
        } else {
            outFile << this_card_ptr->name << ": " << this_top_node.objective;
            outFile << " (";
            double delta = this_top_node.objective - base_objective;
            if (delta > 0) {
                outFile << "+";
            }
            outFile << delta << ")";
            outFile << std::endl;
        }
    }
    outFile.close();
}

// compare the effect on addings cards with the given flags on the given fight
void CompareCards(
        const Node & top_node,
        const CardFlagStruct card_flag,
        const CardFlagStruct bad_flag) {
    std::vector<const Card *> card_list;
    // add base case
    card_list.push_back(nullptr);

    // add cards
    card_list.push_back(&card_anger);
    card_list.push_back(&card_cleave);
    card_list.push_back(&card_clothesline);
    //card_list.push_back(&card_flex);
    card_list.push_back(&card_iron_wave);
    card_list.push_back(&card_perfected_strike);
    card_list.push_back(&card_sword_boomerang);
    card_list.push_back(&card_thunderclap);
    card_list.push_back(&card_twin_strike);
    card_list.push_back(&card_wild_strike);
    card_list.push_back(&card_bludgeon);
    card_list.push_back(&card_whirlwind);
    card_list.push_back(&card_uppercut);
    card_list.push_back(&card_shockwave);

    std::ofstream outFile("card_comparison.txt");
    std::streambuf * oldCoutStreamBuf = std::cout.rdbuf();
    bool base = true;
    double base_objective = 0;
    outFile << top_node.ToString() << std::endl;
    for (auto & this_card_ptr : card_list) {
        Node this_top_node = top_node;
        if (this_card_ptr != nullptr) {
            this_top_node.deck.AddCard(*this_card_ptr);
        }
        this_top_node.InitializeStartingNode();
        TreeStruct tree(this_top_node);
        tree.Expand();
        if (this_card_ptr == nullptr) {
            outFile << "base: " << this_top_node.objective << std::endl;
            base_objective = this_top_node.objective;
            base = false;
        } else {
            outFile << this_card_ptr->name << ": " << this_top_node.objective;
            outFile << " (";
            double delta = this_top_node.objective - base_objective;
            if (delta > 0) {
                outFile << "+";
            }
            outFile << delta << ")";
            outFile << std::endl;
        }
    }
    outFile.close();
}

// compare the effect on addings cards on the given fight
void CompareWatcherCards(const Node & top_node) {
    std::vector<const Card *> card_list;
    // add base case
    card_list.push_back(nullptr);

    // add cards
    card_list.push_back(&card_bowling_bash);
    card_list.push_back(&card_consecrate);
    card_list.push_back(&card_crescendo);
    card_list.push_back(&card_crush_joints);
    card_list.push_back(&card_empty_body);
    card_list.push_back(&card_empty_fist);
    card_list.push_back(&card_flurry_of_blows);
    card_list.push_back(&card_flying_sleeves);
    card_list.push_back(&card_follow_up);
    card_list.push_back(&card_halt);
    card_list.push_back(&card_prostrate);
    card_list.push_back(&card_protect);
    card_list.push_back(&card_sash_whip);
    card_list.push_back(&card_tranquility);

    std::ofstream outFile("card_comparison.txt");
    std::streambuf * oldCoutStreamBuf = std::cout.rdbuf();
    bool base = true;
    double base_objective = 0;
    outFile << top_node.ToString() << std::endl;
    for (auto & this_card_ptr : card_list) {
        Node this_top_node = top_node;
        if (this_card_ptr != nullptr) {
            this_top_node.deck.AddCard(*this_card_ptr);
        }
        this_top_node.InitializeStartingNode();
        TreeStruct tree(this_top_node);
        tree.Expand();
        if (this_card_ptr == nullptr) {
            outFile << "base: " << this_top_node.objective << std::endl;
            base_objective = this_top_node.objective;
            base = false;
        } else {
            outFile << this_card_ptr->name << ": " << this_top_node.objective;
            outFile << " (";
            double delta = this_top_node.objective - base_objective;
            if (delta > 0) {
                outFile << "+";
            }
            outFile << delta << ")";
            outFile << std::endl;
        }
    }
    outFile.close();
}

// compare the effect of upgrading cards on the given fight
void CompareUpgrades(const Node & top_node) {
    std::vector<card_index_t> upgrade_list;

    // add base cards to upgrade
    upgrade_list.push_back(-1);
    for (const auto & deck_item : top_node.deck) {
        if (card_map[deck_item.first]->upgraded_version != nullptr) {
            upgrade_list.push_back(deck_item.first);
        }
    }

    std::ofstream outFile("upgrade_comparison.txt");
    std::streambuf * oldCoutStreamBuf = std::cout.rdbuf();
    bool base = true;
    double base_objective = 0;
    outFile << top_node.ToString() << std::endl;
    for (auto & index : upgrade_list) {
        Node this_top_node = top_node;
        if (index != -1) {
            this_top_node.deck.RemoveCard(index);
            this_top_node.deck.AddCard(*card_map[index]->upgraded_version);
        }
        this_top_node.InitializeStartingNode();
        TreeStruct tree(this_top_node);
        tree.Expand();
        if (index == 65535) {
            outFile << "base: " << this_top_node.objective << std::endl;
            base_objective = this_top_node.objective;
            base = false;
        } else {
            outFile << card_map[index]->upgraded_version->name << ": " <<
                this_top_node.objective;
            outFile << " (";
            double delta = this_top_node.objective - base_objective;
            if (delta > 0) {
                outFile << "+";
            }
            outFile << delta << ")";
            outFile << std::endl;
        }
    }
    outFile.close();
}

// populate deck maps
void PopulateDecks() {

    CardCollectionPtr deck;

    deck.Clear();
    deck.AddCard(card_strike, 5);
    deck.AddCard(card_defend, 4);
    deck.AddCard(card_bash, 1);
    deck_map["starting_ironclad"] = deck;
    deck.AddCard(card_ascenders_bane, 1);
    deck_map["starting_ironclad_cursed"] = deck;

    deck.Clear();
    deck.AddCard(card_strike, 5);
    deck.AddCard(card_defend, 5);
    deck.AddCard(card_survivor, 1);
    deck.AddCard(card_neutralize, 1);
    deck_map["starting_silent"] = deck;
    deck.AddCard(card_ascenders_bane, 1);
    deck_map["starting_silent_cursed"] = deck;

    deck.Clear();
    deck.AddCard(card_strike, 4);
    deck.AddCard(card_defend, 4);
    deck.AddCard(card_zap, 1);
    deck.AddCard(card_dualcast, 1);
    deck_map["starting_defect"] = deck;
    deck.AddCard(card_ascenders_bane, 1);
    deck_map["starting_defect_cursed"] = deck;

    deck.Clear();
    deck.AddCard(card_strike, 4);
    deck.AddCard(card_defend, 4);
    deck.AddCard(card_eruption, 1);
    deck.AddCard(card_vigilance, 1);
    deck_map["starting_watcher"] = deck;
    deck.AddCard(card_ascenders_bane, 1);
    deck_map["starting_watcher_cursed"] = deck;

}

// normalize the string
// (all letters lowercase, remove underscores and spaces)
std::string NormalizeString(const std::string & text) {
    std::string new_text;
    for (auto c : text) {
        if (isalpha(c)) {
            new_text += tolower(c);
        } else if (c == ',') {
            new_text += ',';
        } else if (c == '=') {
            new_text += '=';
        }
    }
    return new_text;
}

// add the second relic to the first relic set
void CombineRelic(RelicStruct & one, const RelicStruct & two) {
    one += two;
    //auto one_ptr = (uint8_t * ) &one;
    //auto two_ptr = (const uint8_t *) &two;
    //for (int i = 0; i < sizeof(one); ++i) {
    //    *one_ptr++ |= *two_ptr++;
    //}
}

// return a relic struct of the given relic string
RelicStruct ParseRelics(std::string text) {
    RelicStruct relic = {0};
    text += ",";
    while (!text.empty()) {
        // get this relic
        std::string relic_name = NormalizeString(text.substr(0, text.find(',')));
        bool found = false;
        for (auto & item : relic_map) {
            if (NormalizeString(item.first) == relic_name) {
                CombineRelic(relic, item.second);
                found = true;
                break;
            }
        }
        if (!found) {
            printf("Unknown relic name: \"%s\"\n", relic_name.c_str());
            exit(1);
        }
        // skip to next comma
        text = text.substr(text.find(',') + 1);
    }
    return relic;
}

// process the given argument, return true if successful
bool ProcessArgument(TreeStruct & tree, std::string original_argument) {
    Node & node = *tree.top_node_ptr;
    std::string argument = NormalizeString(original_argument);
    if (argument.find("=") == std::string::npos) {
        return false;
    }
    // get argument name
    std::string name = argument.substr(0, argument.find("="));
    // argument value
    std::string value = argument.substr(argument.find("=") + 1);
    if (name == "character") {
        bool found = false;
        for (auto & item : character_map) {
            if (value == NormalizeString(item.first)) {
                found = true;
                node.deck = deck_map[character_map[value].deck];
                node.relics = ParseRelics(character_map[value].relics);
                node.max_hp = character_map[value].max_hp;
                node.hp = node.max_hp * 9 / 10;
                break;
            }
        }
        return found;
    } else if (name == "relic" || name == "relics") {
        CombineRelic(node.relics, ParseRelics(value));
    } else if (name == "fight") {
        bool found = false;
        for (auto & item : fight_map) {
            if (value == NormalizeString(item.second.name)) {
                tree.fight_type = item.first;
                printf("Setting fight to %s\n", item.second.name.c_str());
                found = true;
                break;
            }
        }
        return found;
    } else if (name == "maxhp") {
        node.max_hp = atoi(value.c_str());
        printf("Setting max HP to %d\n", (int) node.max_hp);
    } else if (name == "hp") {
        if (value == "full") {
            node.hp = node.max_hp;
        } else {
            node.hp = atoi(value.c_str());;
        }
        printf("Setting HP to %d\n", (int) node.hp);
        if (node.max_hp == 0) {
            node.max_hp = node.hp;
            printf("Setting max HP to %d\n", (int) node.max_hp);
        }
    } else {
        printf("ERROR: argument name \"%s\" not recognized\n", name.c_str());
        return false;
    }
    return true;
}


// entry point
int main(int argc, char ** argv) {

    PopulateDecks();

    // starting node
    Node start_node;
    start_node.hp = 0;
    start_node.max_hp = 0;
    start_node.deck.Clear();
    start_node.relics = {0};

    TreeStruct tree(start_node);

    // if command line arguments given, process them
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (!ProcessArgument(tree, argv[i])) {
                printf("ERROR: count not process argument %s\n", argv[i]);
                exit(1);
            }
        }
    } else {
        //ProcessArgument(tree, "--fight=gremlin_nob");
        //ProcessArgument(tree, "--fight=lagavulin");
        //ProcessArgument(tree, "--fight=blue_slaver");
        //ProcessArgument(tree, "--fight=jaw_worm");
        //ProcessArgument(tree, "--fight=cultist");
        ProcessArgument(tree, "--character=ironclad");
        //ProcessArgument(tree, "--character=silent");
        //ProcessArgument(tree, "--character=defect");
        //ProcessArgument(tree, "--character=watcher");
        //start_node.deck.AddCard(card_flash_of_steel);
        //start_node.deck.AddCard(card_ghostly_armor);
        //start_node.deck.AddCard(card_impervious);
        //start_node.deck.AddCard(card_pummel);
        //start_node.deck.AddCard(card_crush_joints);
        //start_node.deck.AddCard(card_pommel_strike);
        //start_node.deck.AddCard(card_strike_plus);
        //start_node.deck.RemoveCard(card_strike);
        //start_node.deck.AddCard(card_defend_plus, 4);
        //start_node.deck.RemoveCard(card_defend, 4);
        //start_node.deck.UpgradeCard(card_bash);
        //start_node.deck.UpgradeCard(card_strike);
        //start_node.deck.UpgradeAll();
        //start_node.deck.AddCard(card_bash_plus);
        //start_node.deck.RemoveCard(card_bash);
        //start_node.deck.AddCard(card_shrug_it_off);
        //start_node.deck.AddCard(card_demon_form);
        //start_node.deck.AddCard(card_offering);
        //start_node.deck.AddCard(card_sucker_punch);
        //start_node.deck.RemoveCard(card_strike);
        //start_node.deck.RemoveCard(card_ascenders_bane);
        //start_node.deck.AddCard(card_deadly_poison);
        //start_node.deck.RemoveCard(card_ascenders_bane.GetIndex());
        //start_node.deck.RemoveCard(card_vigilance.GetIndex());

        start_node.hp = start_node.max_hp * 9 / 10;

        //start_node.hp = 80;
        //start_node.max_hp = 80;
        tree.fight_type = kFightAct1EliteGremlinNob;
        //tree.fight_type = kFightAct1BurningEliteGremlinNob;
        //tree.fight_type = kFightAct1EasyCultist;
        //tree.fight_type = kFightAct1EasyJawWorm;
        //tree.fight_type = kFightAct1EasyLouses;
        //tree.fight_type = kFightAct1EliteLagavulin;
        //tree.fight_type = kFightTestOneLouse;

        //start_node.deck.AddCard(card_offering);
        //start_node.deck.AddCard(card_blind);
        //start_node.deck.AddCard(card_fiend_fire);
        //start_node.deck.AddCard(card_crush_joints);
        //start_node.deck.AddCard(card_through_violence);

    }

    if (start_node.deck.IsEmpty() || start_node.hp == 0 || tree.fight_type == kFightNone) {
        printf("ERROR: invalid settings\n");
        exit(1);
    }

    start_node.InitializeStartingNode();

    tree.Expand();

    //CompareUpgrades(start_node);

    //CompareRelics(tree, start_node);

    //CompareCards(start_node, {0}, {0});
    //CompareWatcherCards(start_node);

    exit(0);

}
