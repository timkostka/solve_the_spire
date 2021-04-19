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
#include "node.hpp"
#include "fight.hpp"
#include "stopwatch.hpp"

/*

--character=ironclad --relics=pure_water --fight=gremlin_nob --hp=full

--character=silent
--deck=5xstrike,5xdefend
--hp=67/100
--relics=ring_of_the_snake
--cards=5xstrike

--relics=none

*/

//struct TerminalStats {
//    // number of terminal nodes below this
//    uint32_t terminal_node_count;
//    // 
//};

// set of all current card collections
std::set<CardCollection> CardCollectionMap::collection;

struct MobLayout {
    // probability
    double probability;
    // monsters
    Monster mob[MAX_MOBS_PER_NODE];
};

// convert a potentially large unsigned integer into a human readable form
// 1, 10, 100
// 1.00k, 10.0k, 100k,
std::string ToString(std::size_t x) {
    char buffer[32] = {0};
    if (x < 1000) {
        sprintf_s(buffer, sizeof(buffer), "%u", (unsigned int) x);
    } else if (x < 10000) {
        sprintf_s(buffer, sizeof(buffer), "%.2fk", x / 1e3);
    } else if (x < 100000) {
        sprintf_s(buffer, sizeof(buffer), "%.1fk", x / 1e3);
    } else if (x < 1000000) {
        sprintf_s(buffer, sizeof(buffer), "%.0fk", x / 1e3);
    } else if (x < 10000000) {
        sprintf_s(buffer, sizeof(buffer), "%.2fM", x / 1e6);
    } else if (x < 100000000) {
        sprintf_s(buffer, sizeof(buffer), "%.1fM", x / 1e6);
    } else if (x < 1000000000) {
        sprintf_s(buffer, sizeof(buffer), "%.0fM", x / 1e6);
    } else if (x < 10000000000) {
        sprintf_s(buffer, sizeof(buffer), "%.2fB", x / 1e9);
    } else if (x < 100000000000) {
        sprintf_s(buffer, sizeof(buffer), "%.1fB", x / 1e9);
    } else {
        sprintf_s(buffer, sizeof(buffer), "%.0fB", x / 1e9);
    }
    return std::string(buffer);
}

// return all possible monsters from the base
// (variation in HP)
std::vector<std::pair<double, Monster>> GenerateMob(BaseMonster & base) {
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

// generate mob layouts for a given fight
std::list<MobLayout> GenerateAllMobs(FightEnum fight_type) {
    BaseMonster * base_mob[MAX_MOBS_PER_NODE] = {nullptr};
    if (fight_type == kFightAct1EasyCultist) {
        base_mob[0] = &base_mob_cultist;
    } else if (fight_type == kFightAct1EasyJawWorm) {
        base_mob[0] = &base_mob_jaw_worm;
    } else if (fight_type == kFightAct1EasyLouses) {
        base_mob[0] = &base_mob_gremlin_nob;
    } else if (fight_type == kFightAct1EliteLagavulin) {
        base_mob[0] = &base_mob_lagavulin;
    } else if (fight_type == kFightAct1EliteGremlinNob) {
        base_mob[0] = &base_mob_gremlin_nob;
    } else {
        // unknown fight type
        assert(false);
    }
    // find options for all mobs
    std::vector<std::pair<double, Monster>> mob[MAX_MOBS_PER_NODE];
    for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
        if (base_mob[i] == nullptr) {
            mob[i].push_back(std::pair<double, Monster>(1.0, Monster()));
        } else {
            mob[i] = GenerateMob(*base_mob[i]);
        }
    }
    // generate all combinations
    std::list<MobLayout> result;
    uint16_t index[MAX_MOBS_PER_NODE] = {0};
    while (true) {
        // add this combination
        result.push_back(MobLayout());
        MobLayout & layout = *result.rbegin();
        //layout.probability = 1.0;
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            layout.probability *= mob[i][index[i]].first;
            layout.mob[i] = mob[i][index[i]].second;
        }
        // increment
        int i = 0;
        ++index[i];
        while (index[i] == mob[i].size()) {
            index[i] = 0;
            ++i;
            if (i == MAX_MOBS_PER_NODE) {
                return result;
            }
            ++index[i];
        }
    }
}

struct PathObjectiveSort {
    bool operator() (Node * const first, Node * const second) const {
        return (first->path_objective > second->path_objective) ||
            (first->path_objective == second->path_objective &&
                first->index < second->index); //first < second);
    }
};

struct TreeStruct {
    // pointer to top node
    Node * top_node_ptr;
    // list of deleted nodes which we can reuse
    // (in order to avoid thrashing memory with new/delete)
    std::vector<Node *> deleted_nodes;
    // list of all created nodes
    //std::list<Node *> all_nodes;
    // number of nodes created
    std::size_t created_node_count;
    // number of nodes reused
    std::size_t reused_node_count;
    // number of nodes which were expanded
    std::size_t expanded_node_count;
    // nodes which need expanded after a path is formed
    std::set<Node *, PathObjectiveSort> optional_nodes;
    // list of terminal nodes
    // (a terminal node is a node where the battle is over)
    std::set<Node *> terminal_nodes;
    // print stats of current optional node tree
    void PrintOptionalNodeProgress() {
        std::map<unsigned int, unsigned int> unsolved_count;
        for (auto & this_node_ptr : optional_nodes) {
            auto x = this_node_ptr->turn;
            auto it = unsolved_count.find(x);
            if (it == unsolved_count.end()) {
                unsolved_count[x] = 1;
            } else {
                ++(it->second);
            }
        }
        bool first = true;
        for (const auto & item : unsolved_count) {
            if (!first) {
                std::cout << ", ";
            } else {
                first = false;
            }
            std::cout << item.first << ":" << item.second;
        }
        std::cout << std::endl;
    }
    // constructor
    TreeStruct(Node & node) : top_node_ptr(&node) {
        expanded_node_count = 0;
        created_node_count = 0;
        reused_node_count = 0;
    }
    // destructor
    ~TreeStruct() {
        // delete all nodes
        DeleteNodeAndChildren(*top_node_ptr, false);
        // delete unused nodes
        for (auto & node_ptr : deleted_nodes) {
            if (node_ptr != top_node_ptr) {
                delete node_ptr;
            }
        }
        deleted_nodes.clear();
    }
    // add an optional node
    void AddOptionalNode(Node & node) {
        // should have already been evaluated
        assert(node.path_objective == node.GetPathObjective());
        // add it
        optional_nodes.insert(&node);
    }
    // create a new node and return the pointer
    Node & CreateChild(Node & node, bool add_to_optional) {
        static std::size_t next_index = 0;
        ++next_index;
        // if deleted node list has items, reuse the last one
        Node * new_node_ptr = nullptr;
        if (!deleted_nodes.empty()) {
            new_node_ptr = *deleted_nodes.rbegin();
            deleted_nodes.pop_back();
            *new_node_ptr = node;
            ++reused_node_count;
        } else {
            new_node_ptr = new Node(node);
            if (new_node_ptr == nullptr) {
                printf("ERROR: out of memory\n");
                exit(1);
            }
            ++created_node_count;
        }
        Node & new_node = *new_node_ptr;
        new_node.index = next_index;
        new_node.child.clear();
        new_node.parent = &node;
        ++new_node.layer = node.layer + 1;
        node.child.push_back(&new_node);
        if (add_to_optional) {
            new_node.path_objective = new_node.GetPathObjective();
            AddOptionalNode(new_node);
        }
        return new_node;
    }
    // generate mob intents
    void GenerateMobIntents(Node & node) {
        assert(node.generate_mob_intents);
        node.player_choice = false;
        // hold new intent list for all mobs
        std::vector<std::pair<double, uint8_t>> new_intent[MAX_MOBS_PER_NODE];
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            if (!node.monster[i].Exists()) {
                continue;
            }
            new_intent[i] = node.monster[i].GetIntents();
        }
        // now create all child nodes
        uint8_t intent_index[MAX_MOBS_PER_NODE] = {0};
        while (true) {
            // add this node
            Node & new_node = CreateChild(node, true);
            new_node.generate_mob_intents = false;
            new_node.composite_objective = new_node.GetMaxFinalObjective();
            for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
                if (!node.monster[i].Exists()) {
                    continue;
                }
                new_node.monster[i].SelectIntent(
                    new_intent[i][intent_index[i]].second);
                new_node.probability *= new_intent[i][intent_index[i]].first;
            }
            // increment
            bool done = true;
            for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
                ++intent_index[i];
                if (intent_index[i] >= new_intent[i].size()) {
                    intent_index[i] = 0;
                } else {
                    done = false;
                    break;
                }
            }
            if (done) {
                break;
            }
        }
    }
    // draw cards
    void DrawCards(Node & node) {
        assert(node.cards_to_draw > 0);
        node.player_choice = false;
        // if draw pile is empty, move cards from discard to draw pile
        if (node.draw_pile.IsEmpty() && !node.discard_pile.IsEmpty()) {
            // add new node
            Node & new_node = CreateChild(node, true);
            // move discard pile to draw pile
            new_node.draw_pile = new_node.discard_pile;
            new_node.discard_pile.Clear();
            return;
        }
        // draw as many cards as we can
        uint16_t to_draw = std::min(node.cards_to_draw, node.draw_pile.Count());
        if (node.hand.Count() + to_draw > 10) {
            to_draw = 10 - node.hand.Count();
        }
        // if there aren't any cards to draw, then don't
        if (to_draw == 0) {
            // add new node
            Node & new_node = CreateChild(node, true);
            // we have no cards to draw and/or our hand is full
            new_node.cards_to_draw = 0;
            new_node.player_choice = true;
            return;
        }
        // else draw all cards we can and add each as a child node
        auto choices = node.draw_pile.ptr->Select(to_draw);
        for (const auto & choice : choices) {
            // add new node
            Node & new_node = CreateChild(node, true);
            new_node.cards_to_draw -= to_draw;
            new_node.probability *= choice.first;
            new_node.hand.AddCard(choice.second.first);
            new_node.draw_pile = choice.second.second;
            if (new_node.cards_to_draw == 0) {
                new_node.player_choice = true;
            }
            new_node.child.clear();
        }
    }
    // delete this node and any children
    void DeleteNodeAndChildren(Node & node, bool update_terminal = true) {
        // if this is a terminal node, delete it from the terminal node list
        if (update_terminal && node.IsTerminal()) {
            auto it = terminal_nodes.find(&node);
            if (it == terminal_nodes.end()) {
                top_node_ptr->PrintTree(false, &node);
                printf("ERROR: terminal node is missing\n");
            } else {
                terminal_nodes.erase(it);
            }
        }
        // if this node has yet to be expanded, delete it from the optional list
        if (update_terminal && !node.IsTerminal() && node.child.empty()) {
            // TODO: for debug only
            auto it = optional_nodes.find(&node);
            if (it == optional_nodes.end()) {
                printf("ERROR: optional node is missing\n");
                node.parent->PrintTree();
            } else {
                optional_nodes.erase(it);
            }
        }
        // delete children
        for (auto & node_ptr : node.child) {
            DeleteNodeAndChildren(*node_ptr, update_terminal);
        }
        // delete this
        deleted_nodes.push_back(&node);
    }
    // change the tree such that the top node only makes choices which end up
    // at the given node
    // update composite_object/tree_solved of nodes below top node
    void SelectTerminalDecisionPath(Node & top_node, Node & path_node) {
        assert(path_node.tree_solved);
        assert(path_node.IsTerminal());
        Node * node_ptr = &path_node;
        while (node_ptr->parent != &top_node) {
            assert(node_ptr != nullptr);
            assert(node_ptr->parent != nullptr);
            Node & node = *node_ptr;
            Node & parent = *node_ptr->parent;
            assert(parent.player_choice);
            // delete all other children except for this one
            assert(parent.child.size() >= 1);
            if (parent.child.size() != 1) {
                // delete other children
                for (auto & child_ptr : parent.child) {
                    if (child_ptr == &node) {
                        continue;
                    } else {
                        DeleteNodeAndChildren(*child_ptr, false);
                    }
                }
                parent.child[0] = &node;
                parent.child.resize(1);
            } else {
                assert(parent.child[0] == node_ptr);
            }
            // update objectives for non-top nodes
            parent.composite_objective = node.composite_objective;
            assert(node.tree_solved);
            parent.tree_solved = true;
            node_ptr = &parent;
        }
        // delete other children
        for (auto & child_ptr : top_node.child) {
            if (child_ptr == node_ptr) {
                continue;
            } else {
                DeleteNodeAndChildren(*child_ptr, false);
            }
        }
        assert(top_node.child.size() >= 1);
        top_node.child.resize(1);
        top_node.child[0] = node_ptr;
        // add this node to the terminal list
        terminal_nodes.insert(&path_node);
    }
    // add node pointers to set
    // (helper function used during FindPlayerChoices)
    void AddNodesToSet(Node & node, std::set<Node *> & node_set) {
        node_set.insert(&node);
        for (auto this_child : node.child) {
            //node_set.insert(this_child);
            AddNodesToSet(*this_child, node_set);
        }
    }
    // find player choice nodes
    void FindPlayerChoices(Node & top_node/*, bool is_critical*/) {
        top_node.player_choice = true;
        // nodes we must make a decision at
        std::vector<Node *> decision_nodes;
        decision_nodes.push_back(&top_node);
        // nodes at which the player no longer has a choice
        // (e.g. after pressing end turn or after player is dead or all mobs are dead)
        std::vector<Node *> ending_node;
        // expand all decision nodes
        while (!decision_nodes.empty()) {
            // loop through each node we need to expand
            std::vector<Node *> new_decision_nodes;
            for (auto & this_node_ptr : decision_nodes) {
                Node & this_node = *this_node_ptr;
                // add end the turn node
                Node & new_node = CreateChild(this_node, false);
                new_node.layer = top_node.layer + 1;
                new_node.EndTurn();
                // if this path ends the battle at the best possible objective,
                // choose and and don't evaluate other decisions
                if (new_node.IsBattleDone() &&
                        new_node.GetMaxFinalObjective() ==
                        top_node.GetMaxFinalObjective()) {
                    SelectTerminalDecisionPath(top_node, new_node);
                    return;
                }
                ending_node.push_back(&new_node);
                // play all possible unique cards
                for (std::size_t i = 0; i < this_node.hand.ptr->card.size(); ++i) {
                    // alias the card
                    auto & card = *card_map[this_node.hand.ptr->card[i].first];
                    // skip this card if it's unplayable or too expensive
                    if (card.flag.unplayable || card.cost > this_node.energy) {
                        continue;
                    }
                    // play this card
                    if (card.flag.targeted) {
                        // if targeted, cycle among all possible targets
                        for (int m = 0; m < MAX_MOBS_PER_NODE; ++m) {
                            if (!this_node.monster[m].Exists()) {
                                continue;
                            }
                            Node & new_node = CreateChild(this_node, false);
                            new_node.layer = top_node.layer + 1;
                            uint16_t index = this_node.hand.ptr->card[i].first;
                            new_node.hand.RemoveCard(index);
                            new_node.PlayCard(index, m);
                            new_node.SortMobs();
                            // if this is the best possible objective,
                            // don't process any further choices
                            if (new_node.IsBattleDone() &&
                                    new_node.GetMaxFinalObjective() ==
                                    top_node.GetMaxFinalObjective()) {
                                SelectTerminalDecisionPath(top_node, new_node);
                                return;
                            }
                            // add to exhaust or discard pile
                            if (card.flag.exhausts) {
                                new_node.exhaust_pile.AddCard(index);
                            } else {
                                new_node.discard_pile.AddCard(index);
                            }
                            // add new decision point
                            if (new_node.IsBattleDone()) {
                                ending_node.push_back(&new_node);
                            } else {
                                new_decision_nodes.push_back(&new_node);
                            }
                        }
                    } else {
                        Node & new_node = CreateChild(this_node, false);
                        new_node.layer = top_node.layer + 1;
                        uint16_t index = this_node.hand.ptr->card[i].first;
                        new_node.hand.RemoveCard(index);
                        new_node.PlayCard(index);
                        new_node.SortMobs();
                        // if this is the best possible objective,
                        // don't process any further choices
                        if (new_node.IsBattleDone() &&
                                new_node.GetMaxFinalObjective() ==
                                top_node.GetMaxFinalObjective()) {
                            SelectTerminalDecisionPath(top_node, new_node);
                            return;
                        }
                        if (card.flag.exhausts) {
                            new_node.exhaust_pile.AddCard(index);
                        } else {
                            new_node.discard_pile.AddCard(index);
                        }
                        if (new_node.IsBattleDone()) {
                            ending_node.push_back(&new_node);
                        } else {
                            new_decision_nodes.push_back(&new_node);
                        }
                    }
                }
            }
            decision_nodes = new_decision_nodes;
        }
        // find nodes which are equal or worse than another node and remove them
        // once a node is marked bad, don't use it as a comparison
        std::vector<bool> bad_node(ending_node.size(), false);
        for (std::size_t i = 0; i < ending_node.size(); ++i) {
            if (bad_node[i]) {
                continue;
            }
            Node & node_i = *ending_node[i];
            for (std::size_t j = 0; j < ending_node.size(); ++j) {
                if (i == j || bad_node[j]) {
                    continue;
                }
                Node & node_j = *ending_node[j];
                if (node_j.IsWorseOrEqual(node_i)) {
                    bad_node[j] = true;
                }
            }
        }
        // number of nodes to remove from the tree
        uint16_t bad_node_count = 0;
        // number of nodes to keep in the tree
        uint16_t good_node_count = 0;
        // number of nodes in the tree which end in death
        uint16_t dead_node_count = 0;
        Node * good_terminal_node_ptr = nullptr;
        for (std::size_t i = 0; i < ending_node.size(); ++i) {
            Node & node_i = *ending_node[i];
            if (bad_node[i]) {
                ++bad_node_count;
                continue;
            }
            ++good_node_count;
            good_terminal_node_ptr = &node_i;
            if (node_i.IsDead()) {
                ++dead_node_count;
            }
        }
        // if options exist where we don't die, mark options where we die as bad
        if (good_node_count > dead_node_count &&
                dead_node_count > 0) {
            for (std::size_t i = 0; i < ending_node.size(); ++i) {
                Node & node_i = *ending_node[i];
                if (bad_node[i]) {
                    continue;
                }
                if (node_i.IsDead()) {
                    assert(!bad_node[i]);
                    bad_node[i] = true;
                    --good_node_count;
                    --dead_node_count;
                    ++bad_node_count;
                }
            }
        }
        // show choices
        if (show_player_choices) {
            std::cout << "\n" << top_node.ToString() << "\n";
            for (std::size_t i = 0; i < ending_node.size(); ++i) {
                if (bad_node[i]) {
                    printf("X ");
                    std::cout << ending_node[i]->ToString() << "\n";
                }
            }
            for (std::size_t i = 0; i < ending_node.size(); ++i) {
                if (!bad_node[i]) {
                    printf("  ");
                    std::cout << ending_node[i]->ToString() << "\n";
                }
            }
        }
        // at least one node must be good
        assert(good_node_count > 0);
        // remove bad choices and their parents where possible
        for (std::size_t i = 0; i < ending_node.size(); ++i) {
            if (!bad_node[i]) {
                continue;
            }
            // remove this node from its parent and prune empty nodes
            Node * node_ptr = ending_node[i];
            while (node_ptr != &top_node && node_ptr->child.empty()) {
                // delete this node
                deleted_nodes.push_back(node_ptr);
                // remove this node from its parent
                auto & vec = node_ptr->parent->child;
                auto it = std::find(vec.begin(), vec.end(), node_ptr);
                assert(it != vec.end());
                vec.erase(it);
                node_ptr = node_ptr->parent;
            }
        }
        // calculate composite objective of all nodes still in tree
        top_node.CalculateCompositeObjectiveOfChildren();
        for (std::size_t i = 0; i < ending_node.size(); ++i) {
            if (bad_node[i]) {
                continue;
            }
            Node & this_node = *ending_node[i];
            this_node.path_objective = this_node.GetPathObjective();
            if (!this_node.IsBattleDone()) {
                AddOptionalNode(this_node);
            } else {
                terminal_nodes.insert(&this_node);
            }
        }
    }
    // start new battle and generate mobs
    void GenerateBattle(Node & this_node) {
        assert(this_node.turn == 0);
        if (fight_map.find(this_node.fight_type) == fight_map.end()) {
            printf("ERROR: fight_type not found in fight_map\n");
            exit(1);
        }
        std::vector<std::pair<double, std::vector<Monster>>> mob_layouts;
        if (fight_map[this_node.fight_type].base_mob != nullptr) {
            mob_layouts = GenerateFightSingleMob(
                *fight_map[this_node.fight_type].base_mob);
        } else {
            mob_layouts = fight_map[this_node.fight_type].generation_function();
        }
        //std::cout << "Generated " << mob_layouts.size() << " different mob layouts.\n";
        for (auto & layout : mob_layouts) {
            // create one node per mob layout
            Node & new_node = CreateChild(this_node, false);
            new_node.probability *= layout.first;
            if (layout.second.size() > MAX_MOBS_PER_NODE) {
                std::cout << "ERROR: increase MAX_MOBS_PER_NODE to at least "
                    << layout.second.size() << std::endl;
                exit(1);
            }
            for (std::size_t i = 0; i < layout.second.size(); ++i) {
                Monster & mob = new_node.monster[i];
                mob = layout.second[i];
                if (new_node.relics.preserved_insect &&
                    new_node.monster[i].IsElite()) {
                    uint16_t x = mob.hp / 4;
                    mob.hp -= x;
                }
            }
            new_node.StartBattle();
            new_node.composite_objective = new_node.GetMaxFinalObjective();
            new_node.path_objective = new_node.GetPathObjective();
            AddOptionalNode(new_node);
        }
    }
    // print the current tree to a file
    void PrintTreeToFile(std::string filename) {
        printf("Printing tree to %s\n", filename.c_str());
        std::ofstream outFile(filename);
        std::streambuf * oldCoutStreamBuf = std::cout.rdbuf();
        std::cout.rdbuf(outFile.rdbuf());
        top_node_ptr->PrintTree();
        std::cout.rdbuf(oldCoutStreamBuf);
        outFile.close();
    }
    void VerifyCompositeObjective(const Node & node) {
        double x = node.CalculateCompositeObjective();
        if (x != node.composite_objective) {
            printf("ERROR!  composite objective (%g) not as calculated (%g)\n",
                node.composite_objective, x);
            node.PrintTree();
            exit(1);
        }
        for (auto & this_child_ptr : node.child) {
            VerifyCompositeObjective(*this_child_ptr);
        }
    }
    // verify integrity of terminal nodes
    // I'm seeing a problem where the expected final HP is different than the
    // composite objective.  Example:
    //   Tree stats: maxobj=64.5864
    //   Battle setup:
    //   - Starting HP: 72/80
    //   - Starting deck: {11 cards: 5xStrike, 4xDefend, Bash, Ascender's Bane}
    //   Result stats:
    //   - Expected HP change is -5.42
    // We would expect the top node composite objective to be 72 - 5.42 = 66.58
    // but it's 64.58 instead.  This function here is to figure out why.
    void VerifyTerminalNodes() {
        for (auto node_ptr : terminal_nodes) {
            auto & p = node_ptr->probability;
            while (node_ptr != nullptr) {
                const auto & node = *node_ptr;
                double x = node.CalculateCompositeObjective();
                if (x != node.composite_objective) {
                    printf("ERROR!  composite objective (%g) not as calculated (%g)\n",
                        node.composite_objective, x);
                    node.PrintTree();
                    exit(1);
                }
                node_ptr = node_ptr->parent;
            }
        }
    }
    // print stats from the tree
    void PrintTreeStats() {
        VerifyTerminalNodes();
        // debug
        for (auto & node_ptr : terminal_nodes) {
            if (!node_ptr->HasAncestor(*top_node_ptr)) {
                printf("ERROR: terminal node not a descendent of top node\n");
            }
        }
        printf("\nMemory stats:\n");
        printf("- Expanded %lu nodes\n",
            (long unsigned) expanded_node_count);
        printf("- Created %lu nodes\n", (long unsigned) created_node_count);
        printf("- Reused %lu nodes\n", (long unsigned) reused_node_count);
        printf("- Have %lu deleted nodes awaiting reuse\n",
            (long unsigned) deleted_nodes.size());
        printf("- Tree has %lu nodes\n",
            (long unsigned) top_node_ptr->CountNodes());
        printf("- There are %lu terminal nodes\n",
            (long unsigned) terminal_nodes.size());
        // total probability (for check)
        double p_total = 0;
        for (auto & node_ptr : terminal_nodes) {
            p_total += node_ptr->probability;
        }
        if (abs(p_total - 1.0) > 1e-6) {
            printf("ERROR: total probability %g != 1\n", p_total);
        }
        // expected cards drawn each turn
        // (cards drawn is counted when cards_to_draw = 0 and
        // parent.cards_to_draw > 0
        std::vector<std::map<std::size_t, double>> cards_drawn;
        // expected cards played each turn
        std::vector<std::map<std::size_t, double>> cards_played;
        // list of all cards to list
        std::set<uint16_t> card_indices;
        // go through each terminal node
        for (auto node_ptr : terminal_nodes) {
            auto & p = node_ptr->probability;
            for (;
                    node_ptr != nullptr && node_ptr->parent != nullptr;
                    node_ptr = node_ptr->parent) {
                auto & node = *node_ptr;
                auto & parent = *node.parent;
                if (cards_drawn.size() < node.turn) {
                    cards_drawn.resize(node.turn);
                    cards_played.resize(node.turn);
                }
                // count cards drawn
                if (node.cards_to_draw == 0 && parent.cards_to_draw > 0) {
                    for (auto & item : node.hand.ptr->card) {
                        if (cards_drawn[node.turn - 1].find(item.first) ==
                                cards_drawn[node.turn - 1].end()) {
                            cards_drawn[node.turn - 1][item.first] = 0.0;
                        }
                        cards_drawn[node.turn - 1][item.first] += p * item.second;
                        card_indices.insert(item.first);
                    }
                }
                // count cards played
                if (parent.player_choice &&
                        node.parent_decision.type == kDecisionPlayCard) {
                    auto index = node.parent_decision.argument[0];
                    if (cards_played[node.turn - 1].find(index) ==
                            cards_played[node.turn - 1].end()) {
                        cards_played[node.turn - 1][index] = 0.0;
                    }
                    cards_played[node.turn - 1][index] += p;
                    card_indices.insert(index);
                }
            }
        }
        // get average cards drawn/played for entire tree
        std::map<std::size_t, double> total_cards_drawn;
        // expected cards played each turn
        std::map<std::size_t, double> total_cards_played;
        for (std::size_t i = 0; i < cards_drawn.size(); ++i) {
            for (auto & pair : cards_drawn[i]) {
                if (total_cards_drawn.find(pair.first) == total_cards_drawn.end()) {
                    total_cards_drawn[pair.first] = 0.0;
                }
                total_cards_drawn[pair.first] += pair.second;
            }
            for (auto & pair : cards_played[i]) {
                if (total_cards_played.find(pair.first) == total_cards_played.end()) {
                    total_cards_played[pair.first] = 0.0;
                }
                total_cards_played[pair.first] += pair.second;
            }
        }
        // probability of ending up on each final HP
        std::map<int16_t, double> hp_delta;
        double expected_hp_delta = 0.0;
        // chance of battle lasting at least X turns
        std::map<uint16_t, double> turn;
        double expected_turn_count = 0.0;
        double death_chance = 0.0;
        double remaining_mob_hp = 0.0;
        for (auto & node_ptr : terminal_nodes) {
            if (node_ptr->hp != node_ptr->composite_objective) {
                printf("ERROR\n");
            }
            auto & p = node_ptr->probability;
            if (node_ptr->hp == 0) {
                death_chance += p;
                for (auto & mob : node_ptr->monster) {
                    if (mob.Exists()) {
                        remaining_mob_hp += mob.hp * p;
                    }
                }
            }
            {
                int16_t x = (int) node_ptr->hp - (int) top_node_ptr->hp;
                expected_hp_delta += p * x;
                if (hp_delta.find(x) == hp_delta.end()) {
                    hp_delta[x] = p;
                } else {
                    hp_delta[x] += p;
                }
            }
            {
                uint16_t x = node_ptr->turn;
                expected_turn_count += p * x;
                if (turn.find(x) == turn.end()) {
                    turn[x] = p;
                } else {
                    turn[x] += p;
                }
            }
        }
        printf("\nBattle setup:\n");
        printf("- Starting HP: %u/%u\n",
            (unsigned int) top_node_ptr->hp, (unsigned int) top_node_ptr->max_hp);
        printf("- Starting deck: %s\n", top_node_ptr->deck.ToString().c_str());
        printf("- Starting relics: %s\n",
            top_node_ptr->relics.ToString().c_str());
        if (top_node_ptr->turn == 0) {
            printf("- Battle type: %s\n",
                fight_map[top_node_ptr->fight_type].name.c_str());
        } else {
            printf("- Starting mobs are: ");
            bool first = true;
            for (auto & mob : top_node_ptr->monster) {
                if (!mob.Exists()) {
                    continue;
                }
                if (!first) {
                    printf(", ");
                }
                printf("%s", mob.ToString().c_str());
                first = false;
            }
            printf("\n");
        }
        if (normalize_mob_variations) {
            printf("- Variations in mob HP and stats are normalized\n");
        }
        printf("\nResult stats:\n");
        printf("- Expected HP change is %+.6g\n", expected_hp_delta);
        printf("- Min/max HP change of %+d and %+d\n",
            hp_delta.begin()->first, hp_delta.rbegin()->first);
        int16_t low_roll_hp = 32767;
        {
            double x = 0.05;
            auto it = hp_delta.begin();
            while (it != hp_delta.end() && x > 0) {
                low_roll_hp = it->first;
                x -= it->second;
                ++it;
            }
        }
        int16_t high_roll_hp = 32767;
        {
            double x = 0.05;
            auto it = hp_delta.rbegin();
            while (it != hp_delta.rend() && x > 0) {
                high_roll_hp = it->first;
                x -= it->second;
                ++it;
            }
        }
        printf("- Low-roll (5%%) and high-roll (95%%) HP change of %+d and %+d\n",
            low_roll_hp, high_roll_hp);
        // - Expected chance to die is 1% with 24.1 remaining monster HP
        //printf("- Expected HP change is %+g (min %+d, max %+d)\n",
        //    expected_hp_delta, hp_delta.begin()->first, hp_delta.rbegin()->first);
        printf("- Expected fight length is %.3g turns (min %u, max %u)\n",
            expected_turn_count, turn.begin()->first, turn.rbegin()->first);
        if (death_chance != 0) {
            remaining_mob_hp /= death_chance;
            printf("- Expected chance to die is %.3g%% (%.2f remaining mob HP)\n",
                death_chance * 100, remaining_mob_hp);
        }
        printf("\nCards stats:\n");
        std::size_t max_card_name_length = 0;
        for (auto & index : card_indices) {
            const Card & card = *card_map[index];
            if (card.name.size() > max_card_name_length) {
                max_card_name_length = card.name.size();
            }
        }
        // print name
        printf("- Turn");
        for (std::size_t i = 4; i < max_card_name_length; ++i) {
            printf(" ");
        }
        std::vector<double> chance_of_turn(turn.rbegin()->first, 1.0);
        double chance = 1.0;
        for (uint16_t i = 0; i < cards_drawn.size(); ++i) {
            chance_of_turn[i] = chance;
            char buffer[32];
            sprintf_s(buffer, sizeof(buffer), "%d (%.0f%%)", i + 1, chance * 100);
            for (std::size_t i = strlen(buffer); i < 11; ++i) {
                printf(" ");
            }
            printf("%s", std::string(buffer).c_str());
            if (turn.find(i + 1) != turn.end()) {
                chance -= turn[i + 1];
            }
        }
        printf("       Total\n");
        for (auto & index : card_indices) {
            const Card & card = *card_map[index];
            // print name
            printf("- %s", card.name.c_str());
            for (std::size_t i = card.name.size(); i < max_card_name_length; ++i) {
                printf(" ");
            }
            for (uint16_t turn = 0; turn < cards_drawn.size(); ++turn) {
                double drawn = 0.0;
                if (cards_drawn[turn].find(index) != cards_drawn[turn].end()) {
                    drawn = cards_drawn[turn][index];
                }
                double played = 0.0;
                if (cards_played[turn].find(index) != cards_played[turn].end()) {
                    played = cards_played[turn][index];
                }
                drawn /= chance_of_turn[turn];
                played /= chance_of_turn[turn];
                if (drawn != 0.0) {
                    printf("  %3.0f%%/%.2f", played / drawn * 100, drawn);
                } else {
                    printf("        ---");
                }
            }
            printf("   %.2f/%.2f",
                total_cards_played[index], total_cards_drawn[index]);
            printf("\n");
        }
    }
    // delete nodes depending on settings
    void DeleteChildren(Node & node) {
        // TODO
    //    if (!keep_entire_tree_in_memory &&
    //            parent != nullptr &&
    //            parent->parent != nullptr &&
    //            tree_solved) {
    //        node.child.clear();
    //    }
    }
    // update tree and parents if possible
    void UpdateTree(Node * node_ptr) {
        // loop until we can't update any more
        while (node_ptr != nullptr) {
            auto & node = *node_ptr;
            //node_ptr->PrintTree(); // DEBUG
            // doesn't make sense to call this on a solved node
            assert(!node.tree_solved);
            // should have children
            assert(!node.child.empty());
            // if only one child, objective is the same as that child
            if (node.child.size() == 1) {
                auto & child = *node.child[0];
                // if it's different, update and update parent
                if (node.composite_objective != child.composite_objective ||
                    node.tree_solved != child.tree_solved) {
                    node.composite_objective = child.composite_objective;
                    node.tree_solved = child.tree_solved;
                    // prune solved nodes
                    DeleteChildren(node);
                    // update parent
                    node_ptr = node.parent;
                    continue;
                }
                return;
            }
            // if not player choice, objective is probability weighted average of
            // children and tree is solved iff all children are solved
            assert(node.child.size() > 1);
            if (!node.player_choice) {
                double x = node.CalculateCompositeObjective();
                bool solved = node.AreChildrenSolved();
                if (node.composite_objective != x || node.tree_solved != solved) {
                    node.composite_objective = x;
                    node.tree_solved = solved;
                    // TODO: prune solved nodes
                    DeleteChildren(node);
                    node_ptr = node.parent;
                    continue;
                }
                return;
            }
            // if a player choice
            // (1) if all children are solved, choose the best one
            // (2) if no children are solved, objective is the highest child objective
            // (3) if some children are solved, eliminate unsolved paths with
            //     max objectives at or below the max solved objective
            bool solved_children = false;
            double max_solved_objective = 0.0;
            Node * max_solved_objective_ptr = nullptr;
            bool unsolved_children = false;
            double max_unsolved_objective = 0.0;
            for (std::size_t i = 0; i < node.child.size(); ++i) {
                Node & this_child = *node.child[i];
                if (this_child.tree_solved) {
                    if (!solved_children ||
                            this_child.composite_objective > max_solved_objective) {
                        max_solved_objective = this_child.composite_objective;
                        max_solved_objective_ptr = &this_child;
                    }
                    solved_children = true;
                } else {
                    if (!unsolved_children ||
                        this_child.composite_objective > max_unsolved_objective) {
                        max_unsolved_objective = this_child.composite_objective;
                    }
                    unsolved_children = true;
                }
            }
            // (1) if all children solved, choose the best one
            if (!unsolved_children) {
                // delete non-optimal children
                for (auto & child_ptr : node.child) {
                    if (child_ptr == max_solved_objective_ptr) {
                        continue;
                    }
                    DeleteNodeAndChildren(*child_ptr);
                }
                assert(max_solved_objective_ptr != nullptr);
                Node & child = *max_solved_objective_ptr;
                // keep best child
                node.child.resize(1);
                node.child[0] = &child;
                node.tree_solved = true;
                assert(child.composite_objective == max_solved_objective);
                node.composite_objective = max_solved_objective;
                // TODO: prune solved nodes
                DeleteChildren(node);
                node_ptr = node.parent;
                continue;
            }
            // (2) if no children are solved, objective is the highest child objective
            if (!solved_children) {
                if (node.composite_objective != max_unsolved_objective) {
                    node.composite_objective = max_unsolved_objective;
                    node_ptr = node.parent;
                    continue;
                }
                return;
            }
            // (3) if some children are solved, eliminate unsolved paths with
            //     max objectives below the max solved objective and also
            //     eliminate solved paths that are not optimal
            assert(solved_children && unsolved_children);
            {
                std::size_t i = node.child.size();
                while (i > 0) {
                    assert(i > 0);
                    --i;
                    Node & this_child = *node.child[i];
                    if ((this_child.tree_solved &&
                        &this_child != max_solved_objective_ptr) ||
                        (!this_child.tree_solved &&
                            this_child.composite_objective <= max_solved_objective)) {
                        DeleteNodeAndChildren(this_child);
                        node.child.erase(node.child.begin() + i);
                        continue;
                    }
                }
            }
            // if path is now solved, mark it as such
            if (solved_children && node.child.size() == 1) {
                node.tree_solved = true;
            } else {
                assert(max_unsolved_objective > max_solved_objective);
            }
            // if objective changed, update parents
            double x = std::max(max_unsolved_objective, max_solved_objective);
            assert(max_unsolved_objective <= node.composite_objective);
            if (node.tree_solved ||
                    max_unsolved_objective < node.composite_objective) {
                node.composite_objective = x;
                // TODO: prune tree
                DeleteChildren(node);
                node_ptr = node.parent;
                continue;
            }
            break;
        }
    }
    // verify the tree is valid
    bool VerifyNode(Node & node) {
        if (&node == top_node_ptr) {
        }
        //assert(deleted_nodes.find(&node) == deleted_nodes.end());
        bool pass = true;
        if (node.child.empty()) {
            assert(node.tree_solved == node.IsBattleDone());
            if (!node.tree_solved) {
                if (optional_nodes.find(&node) == optional_nodes.end()) {
                    top_node_ptr->PrintTree(false, &node);
                    assert(node.HasAncestor(*top_node_ptr));
                    printf("ERROR: optional node missing\n");
                    pass = false;
                }
            } else {
                if (optional_nodes.find(&node) != optional_nodes.end()) {
                    printf("ERROR: invalid optional node\n");
                    pass = false;
                }
                if (terminal_nodes.find(&node) == terminal_nodes.end()) {
                    printf("ERROR: terminal node missing\n");
                    pass = false;
                }
            }
            return true;
        } else {
            if (optional_nodes.find(&node) != optional_nodes.end()) {
                printf("ERROR: invalid optional node\n");
                pass = false;
            }
            if (terminal_nodes.find(&node) != terminal_nodes.end()) {
                printf("ERROR: invalid terminal node\n");
                pass = false;
            }
        }
        if (node.player_choice && node.tree_solved) {
            assert(node.child.size() == 1);
        }
        double p = 0.0;
        for (auto & ptr : node.child) {
            pass = pass && VerifyNode(*ptr);
            p += ptr->probability;
        }
        // see if it should be in terminal node list
        if (node.IsTerminal()) {
            auto it = terminal_nodes.find(&node);
            if (it == terminal_nodes.end()) {
                printf("ERROR: terminal node not in list\n");
                pass = false;
            }
        } else if (node.child.empty()) {
            auto it = optional_nodes.find(&node);
            if (it == optional_nodes.end()) {
                printf("ERROR: optional node not in list\n");
                pass = false;
            }
        }
        // if all children are solved, this should be solved as well
        if (!node.child.empty()) {
            bool children_solved = true;
            for (auto & it : node.child) {
                if (!it->tree_solved) {
                    children_solved = false;
                }
            }
            if (node.tree_solved && !children_solved) {
                node.PrintTree();
                printf("ERROR: incorrectly marked solved\n");
                pass = false;
            }
            if (!node.tree_solved && children_solved) {
                node.PrintTree();
                printf("ERROR: incorrectly not marked solved\n");
                pass = false;
            }
        }
        return pass;
    }
    // expand this tree
    void Expand() {
        //std::cout << "There are " <<
        //    top_node_ptr->deck.ptr->CountUniqueSubsets() <<
        //    " unique deck subsets\n";
        std::cout << "\n\n\n";
        std::clock_t start_clock = clock();
        std::cout << "Expanding node: " << top_node_ptr->ToString() << "\n\n";
        if (normalize_mob_variations) {
            std::cout << "Mob variations in HP and stats are normalized.\n";
        }
        optional_nodes.clear();
        top_node_ptr->path_objective = top_node_ptr->GetPathObjective();
        optional_nodes.insert(top_node_ptr);
        expanded_node_count = 0;
        std::clock_t next_update = clock();
        bool stats_shown = false;
        bool show_stats = true;

        double update_duration = 1.0;

        // expand nodes until they're all done
        std::size_t iteration = 0;
        while (true) {
            //VerifyCompositeObjective(*top_node_ptr);
            ++iteration;
            // update every second
            stats_shown = false;
            if (show_stats || clock() >= next_update) {
                double p = 0.0;
                for (auto & node_ptr : terminal_nodes) {
                    p += node_ptr->probability;
                }
                std::size_t tree_nodes =
                    1 + created_node_count - deleted_nodes.size();
                std::cout << "Tree stats: maxobj=" <<
                    top_node_ptr->composite_objective <<
                    ", expanded=" <<
                    ToString(expanded_node_count) <<
                    ", generated=" <<
                    ToString(created_node_count + reused_node_count) <<
                    ", stored=" << ToString(tree_nodes);
                printf(", ~%.3g%% complete\n", p * 100);
                //PrintOptionalNodeProgress();
                next_update =
                    clock() + (std::clock_t) (update_duration * CLOCKS_PER_SEC);
                update_duration *= 2;
                if (update_duration > 60) {
                    update_duration = 60;
                }
                stats_shown = true;
                show_stats = false;
            }
            // if we're done, show stats and exit
            if (optional_nodes.empty()) {
                if (!stats_shown) {
                    show_stats = true;
                    continue;
                }
                break;
            }
            // find next node to expand and do it
            Node * this_node_ptr = nullptr;
            this_node_ptr = *optional_nodes.begin();
            optional_nodes.erase(optional_nodes.begin());
            Node & this_node = *this_node_ptr;
            //printf("Expanding (%u): %s\n", (unsigned int) iteration, this_node.ToString().c_str());
            ++expanded_node_count;
            // if just starting, do start of battle initialization
            if (this_node.turn == 0) {
                this_node.player_choice = false;
                GenerateBattle(this_node);
                UpdateTree(&this_node);
                continue;
            }
            // if intents need generated, generate them
            if (this_node.generate_mob_intents) {
                GenerateMobIntents(this_node);
                UpdateTree(&this_node);
                continue;
            }
            // if cards need drawn, draw them
            if (this_node.cards_to_draw) {
                DrawCards(this_node);
                UpdateTree(&this_node);
                continue;
            }
            // else player can play a card or end turn
            //if (iteration == 117) {
            //    this_node.PrintTree();
            //    top_node_ptr->PrintTree();
            //    PrintTreeToFile("tree_before.txt");
            //}
            FindPlayerChoices(this_node);
            //if (iteration == 117) {
            //    this_node.PrintTree();
            //    PrintTreeToFile("tree_middle.txt");
            //}
            UpdateTree(&this_node);
            //if (iteration == 117) {
            //    PrintTreeToFile("tree_after.txt");
            //    this_node.PrintTree();
            //    top_node_ptr->PrintTree();
            //}
        }
        // tree should now be solved
        const double duration = (double) (clock() - start_clock) / CLOCKS_PER_SEC;
        std::cout << "Printing solved tree to tree.txt\n";
        std::cout << "Solution took " << duration << " seconds\n";
        //std::cout << "Deck map contains " <<
        //    CardCollectionMap::collection.size() << " decks\n";
        PrintTreeStats();
        // print solved tree to file
        {
            std::ofstream outFile("tree.txt");
            std::streambuf * oldCoutStreamBuf = std::cout.rdbuf();
            std::cout.rdbuf(outFile.rdbuf());
            std::cout << "Solution took " << duration << " seconds.\n";
            std::cout << "Compiled on " << __DATE__ << " at " __TIME__ << "\n";
            std::cout << "\n";
            if (print_completed_tree_to_file &&
                    top_node_ptr->CountNodes() <= max_nodes_to_print) {
                top_node_ptr->PrintTree();
            }
            std::cout.rdbuf(oldCoutStreamBuf);
            outFile.close();
        }
        VerifyNode(*top_node_ptr);
    }
};

// get list of relics to compare

// compare relics
void CompareRelics(const Node & top_node) {
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
        TreeStruct tree(this_top_node);
        tree.Expand();
        if (base) {
            outFile << this_list.first << ": " << this_top_node.composite_objective << std::endl;
            base_objective = this_top_node.composite_objective;
            base = false;
        } else {
            outFile << this_list.first << ": " << this_top_node.composite_objective;
            outFile << " (";
            double delta = this_top_node.composite_objective - base_objective;
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
void CompareCards(const Node & top_node) {
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
            outFile << "base: " << this_top_node.composite_objective << std::endl;
            base_objective = this_top_node.composite_objective;
            base = false;
        } else {
            outFile << this_card_ptr->name << ": " << this_top_node.composite_objective;
            outFile << " (";
            double delta = this_top_node.composite_objective - base_objective;
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
            outFile << "base: " << this_top_node.composite_objective << std::endl;
            base_objective = this_top_node.composite_objective;
            base = false;
        } else {
            outFile << this_card_ptr->name << ": " << this_top_node.composite_objective;
            outFile << " (";
            double delta = this_top_node.composite_objective - base_objective;
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
    std::vector<uint16_t> upgrade_list;

    // add base cards to upgrade
    upgrade_list.push_back(65535);
    for (auto card : top_node.deck.ptr->card) {
        if (card_map[card.first]->upgraded_version != nullptr) {
            upgrade_list.push_back(card.first);
        }
    }

    std::ofstream outFile("upgrade_comparison.txt");
    std::streambuf * oldCoutStreamBuf = std::cout.rdbuf();
    bool base = true;
    double base_objective = 0;
    outFile << top_node.ToString() << std::endl;
    for (auto & index : upgrade_list) {
        Node this_top_node = top_node;
        if (index != 65535) {
            this_top_node.deck.RemoveCard(index);
            this_top_node.deck.AddCard(*card_map[index]->upgraded_version);
        }
        this_top_node.InitializeStartingNode();
        TreeStruct tree(this_top_node);
        tree.Expand();
        if (index == 65535) {
            outFile << "base: " << this_top_node.composite_objective << std::endl;
            base_objective = this_top_node.composite_objective;
            base = false;
        } else {
            outFile << card_map[index]->upgraded_version->name << ": " <<
                this_top_node.composite_objective;
            outFile << " (";
            double delta = this_top_node.composite_objective - base_objective;
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

    CardCollection deck;

    deck.Clear();
    deck.AddCard(card_strike, 5);
    deck.AddCard(card_defend, 4);
    deck.AddCard(card_bash, 1);
    deck_map["starting_ironclad"] = *(new CardCollection(deck));
    deck.AddCard(card_ascenders_bane, 1);
    deck_map["starting_ironclad_cursed"] = *(new CardCollection(deck));

    deck.Clear();
    deck.AddCard(card_strike, 5);
    deck.AddCard(card_defend, 5);
    deck.AddCard(card_survivor, 1);
    deck.AddCard(card_neutralize, 1);
    deck_map["starting_silent"] = *(new CardCollection(deck));
    deck.AddCard(card_ascenders_bane, 1);
    deck_map["starting_silent_cursed"] = *(new CardCollection(deck));

    deck.Clear();
    deck.AddCard(card_strike, 4);
    deck.AddCard(card_defend, 4);
    deck.AddCard(card_zap, 1);
    deck.AddCard(card_dualcast, 1);
    deck_map["starting_defect"] = *(new CardCollection(deck));
    deck.AddCard(card_ascenders_bane, 1);
    deck_map["starting_defect_cursed"] = *(new CardCollection(deck));

    deck.Clear();
    deck.AddCard(card_strike, 4);
    deck.AddCard(card_defend, 4);
    deck.AddCard(card_eruption, 1);
    deck.AddCard(card_vigilance, 1);
    deck_map["starting_watcher"] = *(new CardCollection(deck));
    deck.AddCard(card_ascenders_bane, 1);
    deck_map["starting_watcher_cursed"] = *(new CardCollection(deck));

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
bool ProcessArgument(Node & node, std::string original_argument) {
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
                // TODO
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
                node.fight_type = item.first;
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
    start_node.fight_type = kFightNone;

    // if command line arguments given, process them
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (!ProcessArgument(start_node, argv[i])) {
                printf("ERROR: count not process argument %s\n", argv[i]);
                exit(1);
            }
        }
    } else {
        //ProcessArgument(start_node, "--character=ironclad");
        //ProcessArgument(start_node, "--character=silent");
        //ProcessArgument(start_node, "--character=defect");
        ProcessArgument(start_node, "--character=watcher");
        //start_node.deck.AddCard(card_crush_joints);
        //start_node.deck.RemoveCard(card_ascenders_bane.GetIndex());
        //start_node.deck.RemoveCard(card_vigilance.GetIndex());

        start_node.hp = start_node.max_hp * 9 / 10;
        //start_node.fight_type = kFightAct1EasyCultist;
        //start_node.fight_type = kFightAct1EasyJawWorm;
        //start_node.fight_type = kFightAct1EasyLouses;
        start_node.fight_type = kFightAct1EliteGremlinNob;
        //start_node.fight_type = kFightAct1EliteLagavulin;
        //start_node.fight_type = kFightTestOneLouse;
    }

    if (start_node.deck.IsEmpty() || start_node.hp == 0 || start_node.fight_type == kFightNone) {
        printf("ERROR: invalid settings\n");
        exit(1);
    }

    start_node.InitializeStartingNode();

    //TreeStruct tree(start_node);
    //tree.Expand();

    CompareUpgrades(start_node);

    //CompareRelics(start_node);

    //CompareWatcherCards(start_node);

    exit(0);

}
