#pragma once

#include <iostream>
#include <list>
#include <vector>
#include <deque>
#include <utility>
#include <ctime>
#include <sstream>
#include <fstream>

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
        snprintf(buffer, sizeof(buffer), "%u", (unsigned int) x);
    } else if (x < 10000) {
        snprintf(buffer, sizeof(buffer), "%.2fk", x / 1e3);
    } else if (x < 100000) {
        snprintf(buffer, sizeof(buffer), "%.1fk", x / 1e3);
    } else if (x < 1000000) {
        snprintf(buffer, sizeof(buffer), "%.0fk", x / 1e3);
    } else if (x < 10000000) {
        snprintf(buffer, sizeof(buffer), "%.2fM", x / 1e6);
    } else if (x < 100000000) {
        snprintf(buffer, sizeof(buffer), "%.1fM", x / 1e6);
    } else if (x < 1000000000) {
        snprintf(buffer, sizeof(buffer), "%.0fM", x / 1e6);
    } else if (x < 10000000000) {
        snprintf(buffer, sizeof(buffer), "%.2fB", x / 1e9);
    } else if (x < 100000000000) {
        snprintf(buffer, sizeof(buffer), "%.1fB", x / 1e9);
    } else {
        snprintf(buffer, sizeof(buffer), "%.0fB", x / 1e9);
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
        printf("ERROR: unknown fight type");
        exit(1);
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
                first < second);
        //first->index < second->index); //first < second);
    }
};

// hold a structure for solving for optimal play decisions
struct TreeStruct {
    // if true, save all nodes, else prune solved nodes as much as possible
    // (value is changed to false when nodes exceed max_nodes_to_store)
    bool keep_all_nodes = true;
    // pointer to top node
    Node * top_node_ptr;
    // fight type
    FightEnum fight_type;
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
    // duration to solve
    double solve_duration_s;
    // expected final hp (populated when solved)
    double final_hp;
    // expected death chance (populated when solved)
    double death_chance;
    // expected remaining mob hp (populated when solved)
    double remaining_mob_hp;
    // constructor
    TreeStruct(Node & node) : top_node_ptr(&node) {
        expanded_node_count = 0;
        created_node_count = 0;
        reused_node_count = 0;
        fight_type = kFightNone;
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
    // create a new node and return a reference to it
    Node & CreateChild(Node & node, bool add_to_optional) {
        //static std::size_t next_index = 0;
        //++next_index;
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
        //new_node.index = next_index;
        new_node.child.clear();
        new_node.parent = &node;
        ++new_node.layer;
        node.child.push_back(&new_node);
        if (add_to_optional) {
            new_node.path_objective = new_node.GetPathObjective();
            AddOptionalNode(new_node);
        }
        return new_node;
    }
    // generate mob intents
    void GenerateMobIntents(Node & node) {
        //assert(node.generate_mob_intents);
        assert(node.pending_action[0].type == kActionGenerateMobIntents);
        //node.player_choice = false;
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
            new_node.PopPendingAction();
            //new_node.generate_mob_intents = false;
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
        //assert(node.cards_to_draw > 0);
        assert(node.pending_action[0].type == kActionDrawCards);
        assert(node.pending_action[0].arg[0] > 0);
        //node.player_choice = false;
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
        card_count_t to_draw = (card_count_t) node.pending_action[0].arg[0];
        if (node.draw_pile.Count() < to_draw) {
            to_draw = node.draw_pile.Count();
        }
        if (node.hand.Count() + to_draw > 10) {
            to_draw = 10 - node.hand.Count();
        }
        // if there aren't any cards to draw, then don't
        if (to_draw == 0) {
            // add new node
            Node & new_node = CreateChild(node, true);
            new_node.PopPendingAction();
            // we have no cards to draw and/or our hand is full
            //new_node.cards_to_draw = 0;
            //new_node.player_choice = true;
            return;
        }
        // else draw all cards we can and add each as a child node
        auto choices = node.draw_pile.Select(to_draw);
        for (const auto & choice : choices) {
            // add new node
            Node & new_node = CreateChild(node, true);
            if (to_draw == new_node.pending_action[0].arg[0]) {
                new_node.PopPendingAction();
            } else {
                new_node.pending_action[0].arg[0] -= to_draw;
            }
            new_node.probability *= choice.first;
            new_node.hand.AddDeck(choice.second.first);
            new_node.draw_pile = choice.second.second;
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
        if (!node.tree_solved && update_terminal && !node.IsTerminal() && node.child.empty()) {
            auto it = optional_nodes.find(&node);
            if (it == optional_nodes.end()) {
                node.parent->PrintTree();
                printf("ERROR: optional node is missing\n");
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
            assert(!parent.HasPendingActions());
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
        top_node.tree_solved = true;
        top_node.composite_objective = path_node.composite_objective;
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
    void FindPlayerChoices(Node & top_node) {
        // nodes we must make a decision at
        std::vector<Node *> decision_nodes;
        decision_nodes.push_back(&top_node);
        const double max_top_objective = top_node.GetMaxFinalObjective();
        // nodes at which the player no longer has a choice
        // (e.g. after pressing end turn or after player is dead or all mobs are dead)
        std::vector<Node *> ending_node;
        // expand all decision nodes
        while (!decision_nodes.empty()) {
            if (show_player_choices) {
                top_node.PrintTree();
            }
            // loop through each node we need to expand
            std::vector<Node *> new_decision_nodes;
            for (auto & this_node_ptr : decision_nodes) {
                Node & this_node = *this_node_ptr;
                // add end the turn node
                {
                    Node & end_turn_node = CreateChild(this_node, false);
                    end_turn_node.EndTurn();
                    // if this path ends the battle at the best possible objective,
                    // choose and and don't evaluate other decisions
                    if (end_turn_node.IsBattleDone() &&
                            end_turn_node.composite_objective ==
                            max_top_objective) {
                        SelectTerminalDecisionPath(top_node, end_turn_node);
                        return;
                    }
                    ending_node.push_back(&end_turn_node);
                }
                // play all possible unique cards
                for (auto & deck_item : this_node.hand) {
                    // alias the card
                    const card_index_t & card_index = deck_item.first;
                    const Card & card = *card_map[card_index];
                    if (&card == &card_offering) {
                        printf("");
                    }
                    // skip this card if it's unplayable or too expensive
                    if (card.flag.unplayable || card.cost > this_node.energy) {
                        continue;
                    }
                    // play this card
                    if (card.flag.targeted) {
                        assert(!card.flag.target_card_in_hand);
                        // if targeted, cycle among all possible targets
                        for (int m = 0; m < MAX_MOBS_PER_NODE; ++m) {
                            if (!this_node.monster[m].Exists()) {
                                continue;
                            }
                            Node & new_node = CreateChild(this_node, false);
                            //card_index_t index = deck_item.first;
                            //const Card & card = *card_map[index];
                            new_node.hand.RemoveCard(card_index);
                            new_node.PlayCard(card_index, m);
                            new_node.SortMobs();
                            // if this is the best possible objective,
                            // don't process any further choices
                            if (new_node.IsBattleDone() &&
                                    new_node.composite_objective ==
                                    max_top_objective) {
                                SelectTerminalDecisionPath(top_node, new_node);
                                return;
                            }
                            // add to exhaust or discard pile
                            if (!new_node.battle_done) {
                                if (card.flag.exhausts) {
                                    new_node.exhaust_pile.AddCard(card_index);
                                } else {
                                    new_node.discard_pile.AddCard(card_index);
                                }
                            }
                            // add new decision point
                            if (new_node.IsBattleDone() || new_node.HasPendingActions()) {
                                ending_node.push_back(&new_node);
                            } else {
                                new_decision_nodes.push_back(&new_node);
                            }
                        }
                    } else if (card.flag.target_card_in_hand) {
                        assert(!card.flag.targeted);
                        assert(&card == &card_armaments);
                        // play on each card that has an upgraded version
                        const auto & deck = this_node.hand.node_ptr->collection.card;
                        for (card_index_t c = 0; c < deck.size(); ++c) {
                            const card_index_t & other_card_index = deck[c].first;
                            const Card & other_card = *card_map[other_card_index];
                            // cannot target itself
                            if (other_card_index == card_index && deck_item.second == 1) {
                                continue;
                            }
                            // target all cards that can be upgraded
                            if (card.upgraded_version != nullptr) {
                                Node & new_node = CreateChild(this_node, false);
                                new_node.hand.RemoveCard(card_index);
                                assert(new_node.hand.CountCard(other_card_index) > 0);
                                new_node.PlayCard(
                                    card_index,
                                    new_node.hand.GetLocalIndex(other_card_index));
                                new_node.SortMobs();
                                // if this is the best possible objective,
                                // don't process any further choices
                                if (new_node.IsBattleDone() &&
                                        new_node.composite_objective ==
                                        max_top_objective) {
                                    SelectTerminalDecisionPath(top_node, new_node);
                                    return;
                                }
                                // add to exhaust or discard pile
                                if (card.flag.exhausts) {
                                    new_node.exhaust_pile.AddCard(card_index);
                                } else {
                                    new_node.discard_pile.AddCard(card_index);
                                }
                                // add new decision point
                                if (new_node.IsBattleDone() || new_node.HasPendingActions()) {
                                    ending_node.push_back(&new_node);
                                } else {
                                    new_decision_nodes.push_back(&new_node);
                                }
                            }
                        }
                        // play on nothing
                        Node & new_node = CreateChild(this_node, false);
                        new_node.hand.RemoveCard(card_index);
                        new_node.PlayCard(card_index, -1);
                        new_node.SortMobs();
                        // if this is the best possible objective,
                        // don't process any further choices
                        if (new_node.IsBattleDone() &&
                                new_node.composite_objective ==
                                max_top_objective) {
                            SelectTerminalDecisionPath(top_node, new_node);
                            return;
                        }
                        // add to exhaust or discard pile
                        if (card.flag.exhausts) {
                            new_node.exhaust_pile.AddCard(card_index);
                        } else {
                            new_node.discard_pile.AddCard(card_index);
                        }
                        // add new decision point
                        if (new_node.IsBattleDone() || new_node.HasPendingActions()) {
                            ending_node.push_back(&new_node);
                        } else {
                            new_decision_nodes.push_back(&new_node);
                        }
                    } else {
                        Node & new_node = CreateChild(this_node, false);
                        new_node.hand.RemoveCard(card_index);
                        new_node.PlayCard(card_index);
                        new_node.SortMobs();
                        // if this is the best possible objective,
                        // don't process any further choices
                        if (new_node.IsBattleDone() &&
                                new_node.composite_objective ==
                                max_top_objective) {
                            SelectTerminalDecisionPath(top_node, new_node);
                            return;
                        }
                        if (card.flag.exhausts) {
                            new_node.exhaust_pile.AddCard(card_index);
                        } else {
                            new_node.discard_pile.AddCard(card_index);
                        }
                        if (new_node.IsBattleDone() || new_node.HasPendingActions()) {
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
        std::vector<bool> bad_node(ending_node.size(), false);
        // if we have multiple nodes that end up dead, mark all except the best one as bad
        Node * best_dead_node = nullptr;
        std::size_t best_dead_node_index = 0;
        for (std::size_t i = 0; i < ending_node.size(); ++i) {
            Node & node = *ending_node[i];
            if (!node.IsDead()) {
                continue;
            }
            if (best_dead_node == nullptr ||
                    node.composite_objective > best_dead_node->composite_objective) {
                if (best_dead_node != nullptr) {
                    bad_node[best_dead_node_index] = true;
                }
                best_dead_node = &node;
                best_dead_node_index = i;
            } else {
                bad_node[i] = true;
            }
        }
        // once a node is marked bad, don't use it as a comparison
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
                    //std::cout << "Node " << j << " <= Node " << i;
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
        // we should have eliminated all but one dead node
        assert(dead_node_count <= 1);
        // if options exist where we don't die, mark options where we die as bad
        if (always_avoid_dying &&
                good_node_count > dead_node_count &&
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
        //top_node.PrintTree();
        top_node.CalculateCompositeObjectiveOfChildren();
        //top_node.PrintTree();
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
        assert(this_node.pending_action[0].type == kActionGenerateBattle);
        if (fight_map.find(fight_type) == fight_map.end()) {
            printf("ERROR: fight_type not found in fight_map\n");
            exit(1);
        }
        std::vector<std::pair<double, std::vector<Monster>>> mob_layouts;
        if (fight_map[fight_type].base_mob != nullptr) {
            mob_layouts = GenerateFightSingleMob(
                *fight_map[fight_type].base_mob);
        } else {
            mob_layouts = fight_map[fight_type].generation_function();
        }
        //std::cout << "Generated " << mob_layouts.size() << " different mob layouts.\n";
        for (auto & layout : mob_layouts) {
            // create one node per mob layout
            Node & new_node = CreateChild(this_node, false);
            new_node.PopPendingAction();
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
    // return the profile line for this tree
    std::string GetProfileLine() {
        char profile_line[256] = {0};
        char buffer[80];
        time_t rawtime;
        struct tm * timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer, sizeof(buffer), "%Y-%m-%d   %H:%M:%S", timeinfo);
        snprintf(profile_line, sizeof(profile_line),
            "%s   %9s   %8s   %6s   %6.3f s\n",
            buffer,
            ToString(created_node_count + reused_node_count).c_str(),
            ToString(expanded_node_count).c_str(),
            ToString(top_node_ptr->CountNodes()).c_str(),
            solve_duration_s);
        return std::string(profile_line);
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
                std::size_t turn_index = (std::size_t) node.turn - 1;
                // if (node.cards_to_draw == 0 && parent.cards_to_draw > 0) {
                if (node.pending_action[0].type != kActionDrawCards &&
                    parent.pending_action[0].type == kActionDrawCards) {
                    for (auto & deck_item : node.hand) {
                        if (cards_drawn[turn_index].find(deck_item.first) ==
                            cards_drawn[turn_index].end()) {
                            cards_drawn[turn_index][deck_item.first] = 0.0;
                        }
                        cards_drawn[turn_index][deck_item.first] += p * deck_item.second;
                        card_indices.insert(deck_item.first);
                    }
                }
                // count cards played
                if (!parent.HasPendingActions() &&
                    node.parent_decision.type == kDecisionPlayCard) {
                    auto index = node.parent_decision.argument[0];
                    if (cards_played[turn_index].find(index) ==
                        cards_played[turn_index].end()) {
                        cards_played[turn_index][index] = 0.0;
                    }
                    cards_played[turn_index][index] += p;
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
        final_hp = 0.0;
        // chance of battle lasting at least X turns
        std::map<uint16_t, double> turn;
        double expected_turn_count = 0.0;
        death_chance = 0.0;
        double no_loss_chance = 0.0;
        remaining_mob_hp = 0.0;
        for (auto & node_ptr : terminal_nodes) {
            // this is no longer true since we use mob hp in the calculation as well
            /*if (node_ptr->hp != node_ptr->composite_objective) {
                printf("ERROR\n");
            }*/
            auto & p = node_ptr->probability;
            final_hp += p * node_ptr->hp;
            if (node_ptr->hp >= top_node_ptr->hp) {
                no_loss_chance += p;
            }
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
                fight_map[fight_type].name.c_str());
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
        printf("- Expected final HP of %.6g\n", top_node_ptr->hp + expected_hp_delta);
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
        if (death_chance > 0.0) {
            remaining_mob_hp /= death_chance;
            printf("- Chance to die is %.3g%% (%.2f remaining mob HP)\n",
                100 * death_chance, remaining_mob_hp);
        }
        if (no_loss_chance > 0.0) {
            printf("- Chance not to lose life is %.3g%%\n", 100 * no_loss_chance);
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
            snprintf(buffer, sizeof(buffer), "%d (%.0f%%)", i + 1, chance * 100);
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
        //top_node_ptr->PrintTree();
    }
    // delete nodes depending on settings
    void DeleteChildren(Node & node) {
        // update flag
        if (keep_all_nodes && created_node_count > max_nodes_to_store) {
            keep_all_nodes = false;
            printf("Note: no longer keeping all nodes\n");
            // TODO: go through tree and delete children of solved nodes
        }
        // if we're saving all nodes, just return
        if (keep_all_nodes) {
            return;
        }
        // always keep the top node
        if (node.parent == nullptr) {
            return;
        }
        // if solved, delete all children
        if (node.tree_solved) {
            for (auto & child_ptr : node.child) {
                DeleteNodeAndChildren(*child_ptr);
            }
            node.child.clear();
        }
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
                    DeleteChildren(node);
                    node_ptr = node.parent;
                    continue;
                }
                return;
            }
            // if not player choice, objective is probability weighted average of
            // children and tree is solved iff all children are solved
            assert(node.child.size() > 1);
            if (node.HasPendingActions()) {
                double x = node.CalculateCompositeObjective();
                bool solved = node.AreChildrenSolved();
                if (node.composite_objective != x || node.tree_solved != solved) {
                    node.composite_objective = x;
                    node.tree_solved = solved;
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
        bool pass = true;
        // if tree is solved and it's a player choice node, it must have exactly one child
        if (node.tree_solved && !node.IsBattleDone() && !node.HasPendingActions()) {
            if (node.child.size() != 1) {
                pass = false;
                node.PrintTree();
                printf("ERROR: solved choice node has more than one child\n");
            }
        }
        //if (node.HasChildren() && node.AreChildrenSolved()) {
        //    if (!node.tree_solved) {
        //        pass = false;
        //        node.PrintTree();
        //        printf("ERROR: parent not marked solved\n");
        //    }
        //}
        //assert(deleted_nodes.find(&node) == deleted_nodes.end());
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
        if (!node.HasPendingActions() && node.tree_solved) {
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
            if (verify_all_expansions) {
                if (!VerifyNode(*top_node_ptr)) {
                    exit(1);
                }
            }
            //std::cout << "\n";
            //top_node_ptr->PrintTree();
            //VerifyCompositeObjective(*top_node_ptr);
            ++iteration;
            // update every second
            stats_shown = false;
            if (show_stats || clock() >= next_update) {
                auto est_obj_result = top_node_ptr->EstimateFinalObjective();
                std::size_t tree_nodes =
                    1 + created_node_count - deleted_nodes.size();
                std::cout << "Tree stats: maxobj=" <<
                    top_node_ptr->composite_objective;
                if (est_obj_result.first > 0.0) {
                    std::cout << ", estobj=" <<
                        (est_obj_result.second / est_obj_result.first);
                }
                std::cout <<
                    ", expanded=" <<
                    ToString(expanded_node_count) <<
                    ", generated=" <<
                    ToString(created_node_count + reused_node_count) <<
                    ", stored=" << ToString(tree_nodes);
                printf(", %.3g%% complete\n",
                    top_node_ptr->GetSolvedCompletionPercent() * 100);
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
            // do next preaction
            if (this_node.pending_action[0].type != kActionNone) {
                if (this_node.pending_action[0].type == kActionGenerateBattle) {
                    //this_node.player_choice = false;
                    GenerateBattle(this_node);
                    UpdateTree(&this_node);
                } else if (this_node.pending_action[0].type == kActionGenerateMobIntents) {
                    GenerateMobIntents(this_node);
                    UpdateTree(&this_node);
                } else if (this_node.pending_action[0].type == kActionDrawCards) {
                    DrawCards(this_node);
                    UpdateTree(&this_node);
                } else {
                    printf("ERROR: unexpected preaction type\n");
                    exit(1);
                }
                continue;
            }
            // play a card or end the turn
            //this_node.PrintTree();
            FindPlayerChoices(this_node);
            //this_node.PrintTree();
            if (this_node.parent != nullptr) {
                //this_node.parent->PrintTree();
                UpdateTree(this_node.parent);
                //this_node.parent->PrintTree();
            }
        }
        // tree should now be solved
        const double duration = (double) (clock() - start_clock) / CLOCKS_PER_SEC;
        std::cout << "Solution took " << duration << " seconds\n";
        PrintTreeStats();
        // print solved tree to file
        if (print_completed_tree_to_file &&
            top_node_ptr->CountNodes() <= max_nodes_to_print) {
            std::cout << "Printing solved tree to tree.txt\n";
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
        solve_duration_s = duration;
        VerifyNode(*top_node_ptr);
        printf("\nPROFILE: %s\n", GetProfileLine().c_str());
    }
};

