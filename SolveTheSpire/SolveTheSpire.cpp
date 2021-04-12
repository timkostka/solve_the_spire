#include <iostream>
#include <list>
#include <vector>
#include <deque>
#include <utility>
#include <ctime>
#include <sstream>
#include <fstream>

#include "defines.h"
#include "card_collection.hpp"
#include "node.hpp"
#include "fight.hpp"
#include "stopwatch.hpp"

/*

When we fill in a tree, we first try to find a path for all branches, then we
fill in the tree and select better choices when available.

When a choice comes back with multiple terminal nodes, we mark the one with the
highest to be expanded and put the others in reserve.  When all paths have a
solution, we go back to these reserve branches and expand them looking for
better options.

*/

/*

Final stats we need:
- probability of fight lasting X rounds
- expected times casting and drawing each card
- final hp statistics (probability of each HP)
- final composite_objective statistics (probability of each?)
- final cumulative hp of mobs (in the event we die)


Example final stats:

Fight stats:
- Expected fight length is X rounds (min of X, max of X)
- Expected HP change is -7.32153 (min of -21, max of +6)
- Battle is lost 0% of the time (expected remaining mob HP is X)

Card stats:
- Strike: played X/Y times (78%)
- Defend: played X/Y times (27%)
- Bash: played X/Y times (25.321%)
- Ascender's Bane: unplayable, drawn 1 times

Tree stats:
- Expanded a total of X nodes.
- Max of X nodes in use.
- Completed tree contains X nodes including X terminal nodes.
- Expected final objective of X
  - min of X
  - 1% threshold of X
  - 5% threshold of X
  - 25% threshold of X
  - median of X
  - 75% threshold of X
  - 95% threshold of X
  - max of X

*/

struct TerminalStats {
    // number of terminal nodes below this
    uint32_t terminal_node_count;
    // 
};


// if true, will use average mob HP (saves much time)
//const bool use_average_mob_hp = true;

// set of all current card collections
std::set<CardCollection> CardCollectionMap::collection;

struct MobLayout {
    // probability
    double probability;
    // monsters
    Monster mob[MAX_MOBS_PER_NODE];
};

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
    // nodes which need expanded first
    // (since all must be evaluated, no need to sort them)
    //std::list<Node *> critical_nodes;
    // nodes which need expanded after a path is formed
    std::set<Node *, PathObjectiveSort> optional_nodes;
    //std::list<Node *> optional_nodes;
    // list of terminal nodes
    // (a terminal node is a node where the battle is over)
    std::set<Node *> terminal_nodes;
    // number of terminal nodes found
    //uint32_t terminal_node_count;
    // number of nodes expanded with multiple choices
    // (this is not incremented if one choice is strictly better)
    //uint32_t node_choice_count;
    // print stats of current optional node tree
    void PrintOptionalNodeProgress() {
        std::map<unsigned int, unsigned int> unsolved_count;
        for (auto & this_node_ptr : optional_nodes) {
            auto x = this_node_ptr->turn; // ->CountLevelsBelow(*top_node_ptr);
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
        //node_choice_count = 0;
        created_node_count = 0;
        reused_node_count = 0;
    }
    // destructor
    ~TreeStruct() {
        //assert(critical_nodes.empty());
        //assert(optional_nodes.empty());
        // delete all nodes except for the top
        //for (auto & node_ptr : all_nodes) {
        //    if (node_ptr != top_node_ptr) {
        //        delete node_ptr;
        //    }
        //}
        //all_nodes.clear();
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
    Node & CreateChild(Node & node, /*bool add_to_critical,*/ bool add_to_optional) {
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
        //all_nodes.push_back(&new_node);
        ++new_node.layer = node.layer + 1;
        node.child.push_back(&new_node);
        /*if (add_to_critical) {
            critical_nodes.push_back(&new_node);
        } else*/ if (add_to_optional) {
            new_node.path_objective = new_node.GetPathObjective();
            AddOptionalNode(new_node);
            //optional_nodes.push_back(&new_node);
        }
        return new_node;
    }
    // prune orphaned nodes from the tree
    //void Prune() {
    //    const Node & top_node = *top_node_ptr;
    //    Stopwatch watch;
    //    // prune optional_nodes
    //    //std::size_t prune_count_1 = 0;
    //    {
    //        auto it = optional_nodes.begin();
    //        while (it != optional_nodes.end()) {
    //            Node & node = **it;
    //            if (!node.HasAncestor(top_node)) {
    //                it = optional_nodes.erase(it);
    //                //++prune_count_1;
    //            } else {
    //                ++it;
    //            }
    //        }
    //    }
    //    // prune all_nodes
    //    // go in reverse order to ensure children are erased before parents
    //    std::size_t prune_count = 0;
    //    {
    //        auto it = all_nodes.end();
    //        while (it != all_nodes.begin()) {
    //            --it;
    //            Node & node = **it;
    //            if (!node.HasAncestor(top_node)) {
    //                delete &node;
    //                auto & it2 = it;
    //                it = all_nodes.erase(it);
    //                ++prune_count;
    //            }
    //        }
    //    }
    //    //printf("Pruned %.3g%% (%u/%u) of nodes in %.3g seconds\n",
    //    //    100.0 * prune_count / (all_nodes.size() + prune_count),
    //    //    (unsigned int) prune_count,
    //    //    (unsigned int) (all_nodes.size() + prune_count),
    //    //    watch.GetTime());
    //    //printf("Pruned %u (%.3g%%) nodes from optional_nodes\n",
    //    //    prune_count_1,
    //    //    100.0 * prune_count_1 / (optional_nodes.size() + prune_count_1));
    //    //printf("Pruned %u (%.3g%%) nodes from all_nodes\n",
    //    //    prune_count_2,
    //    //    100.0 * prune_count_2 / (all_nodes.size() + prune_count_2));
    //    //printf("Pruning took %.3g seconds\n", watch.GetTime());
    //}
    // generate mob intents
    void GenerateMobIntents(Node & node/*, bool is_critical*/) {
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
            //Node & new_node = CreateChild(node, is_critical, !is_critical);
            Node & new_node = CreateChild(node, true);
            new_node.generate_mob_intents = false;
            //new_node.probability = 1.0;
            //new_node.child.clear();
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
    void DrawCards(Node & node/*, bool is_critical*/) {
        assert(node.cards_to_draw > 0);
        node.player_choice = false;
        // if draw pile is empty, move cards from discard to draw pile
        if (node.draw_pile.IsEmpty() && !node.discard_pile.IsEmpty()) {
            // add new node
            //Node & new_node = CreateChild(node, is_critical, !is_critical);
            Node & new_node = CreateChild(node, true);
            //new_node.probability = 1.0;
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
            //Node & new_node = CreateChild(node, is_critical, !is_critical);
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
            //Node & new_node = CreateChild(node, is_critical, !is_critical);
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
        //if (expanded_node_count == 109) {
        //    top_node.parent->PrintTree();
        //    printf("");
        //}
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
        //if (expanded_node_count == 109) {
        //    top_node.parent->PrintTree();
        //    printf("");
        //}
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
    // delete nodes which are no longer in tree
    // (helper function used during FindPlayerChoices)
    //void DeleteUnusedPlayerChoices(
    //        Node & top_node, Node * const last_stored_node_ptr) {
    //    std::size_t deleted_count = 0;
    //    std::size_t kept_count = 0;
    //    // store used nodes into set
    //    std::set<Node *> used_node;
    //    AddNodesToSet(top_node, used_node);
    //    // go through nodes and delete unused ones
    //    auto it = all_nodes.end();
    //    while (*(--it) != last_stored_node_ptr) {
    //        Node * node_ptr = *it;
    //        if (used_node.find(node_ptr) == used_node.end()) {
    //            deleted_nodes.push_back(node_ptr);
    //            //delete node_ptr;
    //            it = all_nodes.erase(it);
    //        }
    //    }
    //}
    // 
    // find player choice nodes
    void FindPlayerChoices(Node & top_node/*, bool is_critical*/) {
        top_node.player_choice = true;
        // hold pointer to last item in all_nodes
        //Node * const last_stored_node_ptr = *all_nodes.rbegin();
        //std::cout << "\nExpanding: " << ToString() << "\n";
        // nodes we must make a decision at
        std::vector<Node *> decision_nodes;
        decision_nodes.push_back(&top_node);
        // nodes at which the player no longer has a choice
        // (e.g. after pressing end turn or after player is dead or all mobs are dead)
        std::vector<Node *> ending_node;
        //bool dead_node = false;
        // expand all decision nodes
        while (!decision_nodes.empty()) {
            // loop through each node we need to expand
            std::vector<Node *> new_decision_nodes;
            for (auto & this_node_ptr : decision_nodes) {
                Node & this_node = *this_node_ptr;
                // add end the turn node
                //Node & new_node = CreateChild(this_node, false, false);
                Node & new_node = CreateChild(this_node, false);
                new_node.layer = top_node.layer + 1;
                new_node.EndTurn();
                //new_node.probability = 1.0;
                //if (new_node.hp == 0) {
                //    dead_node = true;
                //    //printf("Dead!\n");
                //}
                // if this path ends the battle at the best possible objective,
                // choose and and don't evaluate other decisions
                if (new_node.IsBattleDone() &&
                        new_node.GetMaxFinalObjective() ==
                        top_node.GetMaxFinalObjective()) {
                    SelectTerminalDecisionPath(top_node, new_node);
                    // todo: delete other nodes
                    //DeleteUnusedPlayerChoices(top_node, last_stored_node_ptr);
                    //top_node.UpdateParents();
                    return;
                }
                ending_node.push_back(&new_node);
                // play all possible unique cards
                for (std::size_t i = 0; i < this_node.hand.ptr->card.size(); ++i) {
                    // alias the card
                    auto & card = *card_index[this_node.hand.ptr->card[i].first];
                    // skip this card if it's unplayable or too expensive
                    if (card.IsUnplayable() || card.cost > this_node.energy) {
                        continue;
                    }
                    // play this card
                    if (card.IsTargeted()) {
                        // if targeted, cycle among all possible targets
                        for (int m = 0; m < MAX_MOBS_PER_NODE; ++m) {
                            if (!this_node.monster[m].Exists()) {
                                continue;
                            }
                            //Node & new_node = CreateChild(this_node, false, false);
                            Node & new_node = CreateChild(this_node, false);
                            new_node.layer = top_node.layer + 1;
                            //new_node.probability = 1.0;
                            uint16_t index = this_node.hand.ptr->card[i].first;
                            new_node.hand.RemoveCard(index);
                            new_node.PlayCard(index, m);
                            // if this is the best possible objective,
                            // don't process any further choices
                            if (new_node.IsBattleDone() &&
                                    new_node.GetMaxFinalObjective() ==
                                    top_node.GetMaxFinalObjective()) {
                                SelectTerminalDecisionPath(top_node, new_node);
                                /*DeleteUnusedPlayerChoices(
                                    top_node, last_stored_node_ptr);*/
                                //top_node.UpdateParents();
                                return;
                            }
                            // add to exhaust or discard pile
                            if (card.Exhausts()) {
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
                        //Node & new_node = CreateChild(this_node, false, false);
                        Node & new_node = CreateChild(this_node, false);
                        new_node.layer = top_node.layer + 1;
                        //new_node.probability = 1.0;
                        uint16_t index = this_node.hand.ptr->card[i].first;
                        new_node.hand.RemoveCard(index);
                        new_node.PlayCard(index);
                        // if this is the best possible objective,
                        // don't process any further choices
                        if (new_node.IsBattleDone() &&
                                new_node.GetMaxFinalObjective() ==
                                top_node.GetMaxFinalObjective()) {
                            SelectTerminalDecisionPath(top_node, new_node);
                            /*DeleteUnusedPlayerChoices(
                                top_node, last_stored_node_ptr);*/
                            //top_node.UpdateParents();
                            return;
                        }
                        if (card.Exhausts()) {
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
        //if (false && dead_node) {
        //    top_node.PrintTree();
        //}
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
                    //printf("Node %u <= node %u\n", j, i);
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
        top_node.CalculateCompositeObjectiveIncludingChildren();
        // TODO: tree_solved is not updated here, but probably needs to be
        // if fight ever ends but max objective isn't reached, this will cause issues
        // for example, if fight ends with a thorns attack
        //{
        //    auto it = all_nodes.end();
        //    while (*(--it) != last_stored_node_ptr) {
        //        Node & node = **it;
        //        // if node is no longer in the tree, delete it
        //        //if (!node.HasAncestor(top_node)) {
        //        //    delete &node;
        //        //    it = all_nodes.erase(it);
        //        //    continue;
        //        //}
        //        // at this point, if node has no children, it should be listed
        //        // as a good node (i.e. not listed as a bad node)
        //        // calculate objective
        //        node.composite_objective = node.CalculateCompositeObjective();
        //        // propagate solved tree state
        //        if (node.child.size() == 1 && node.child[0]->tree_solved) {
        //            node.tree_solved = true;
        //        }
        //    }
        //}
        // if we're on a critical path and end of battle was found on a good
        // node, all paths which don't end the fight are optional
        //if (is_critical) {
        //    for (std::size_t i = 0; i < terminal_node.size(); ++i) {
        //        if (bad_node[i]) {
        //            continue;
        //        }
        //        if (terminal_node[i]->IsBattleDone()) {
        //            is_critical = false;
        //            break;
        //        }
        //    }
        //}
        // if not on a critical path, all decisions are optional
        //if (!is_critical) {
            for (std::size_t i = 0; i < ending_node.size(); ++i) {
                if (bad_node[i]) {
                    continue;
                }
                Node & this_node = *ending_node[i];
                this_node.path_objective = this_node.GetPathObjective();
                if (!this_node.IsBattleDone()) {
                    //optional_nodes.push_back(&this_node);
                    AddOptionalNode(this_node);
                } else {
                    terminal_nodes.insert(&this_node);
                    //printf("TODO: add terminal node?\n");
                }
            }
            return;
        //}
        // if we're on a critical path, select choice with best path objective
        // to add to critical path nodes and add others to optional nodes
        //double best_objective = 0.0;
        //std::size_t best_objective_index = -1;
        //for (std::size_t i = 0; i < terminal_node.size(); ++i) {
        //    if (bad_node[i]) {
        //        continue;
        //    }
        //    assert(!terminal_node[i]->IsBattleDone());
        //    double objective = terminal_node[i]->GetPathObjective();
        //    terminal_node[i]->path_objective = objective;
        //    if (best_objective_index == -1) {
        //        best_objective = objective;
        //        best_objective_index = i;
        //    } else if (objective > best_objective) {
        //        //optional_nodes.push_back(terminal_node[best_objective_index]);
        //        AddOptionalNode(*terminal_node[best_objective_index]);
        //        best_objective = objective;
        //        best_objective_index = i;
        //    } else {
        //        //optional_nodes.push_back(terminal_node[i]);
        //        AddOptionalNode(*terminal_node[i]);
        //    }
        //}
        //assert(best_objective_index != -1);
        //critical_nodes.push_back(terminal_node[best_objective_index]);
    }
    // return total number of orphaned nodes
    //std::size_t CountOrphanedNodes() {
    //    std::size_t result = 0;
    //    for (auto & it : all_nodes) {
    //        if (!it->HasAncestor(*top_node_ptr)) {
    //            ++result;
    //        }
    //    }
    //    return result;
    //}
    // start new battle and generate mobs
    void GenerateBattle(Node & this_node) {
        assert(this_node.turn == 0);
        //for (auto & layout : GenerateAllMobs(this_node.fight_type)) {
        //    // create one node per mob layout
        //    Node & new_node = CreateChild(this_node, true, false);
        //    new_node.probability = layout.probability;
        //    for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
        //        Monster & mob = new_node.monster[i];
        //        mob = layout.mob[i];
        //        if (new_node.relics.preserved_insect &&
        //            new_node.monster[i].IsElite()) {
        //            uint16_t x = mob.hp / 4;
        //            mob.hp -= x;
        //        }
        //    }
        //    new_node.StartBattle();
        //    new_node.composite_objective = new_node.GetMaxFinalObjective();
        //}
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
        std::cout << "Generated " << mob_layouts.size() << " different mob layouts.\n";
        for (auto & layout : mob_layouts) {
            // create one node per mob layout
            //Node & new_node = CreateChild(
            //    this_node, false, false);
            Node & new_node = CreateChild(this_node, false);
            new_node.probability *= layout.first;
            if (layout.second.size() > MAX_MOBS_PER_NODE) {
                std::cout << "ERROR: increase MAX_MOBS_PER_NODE to at least "
                    << layout.second.size() << std::endl;
                exit(1);
            }
            for (int i = 0; i < layout.second.size(); ++i) {
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
            //if (evaluate_critical_path) {
            //    critical_nodes.push_back(&new_node);
            //} else {
            new_node.path_objective = new_node.GetPathObjective();
            AddOptionalNode(new_node);
            //}
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
    // print stats from the tree
    //    Fight stats :
    //    - Expected fight length is X rounds (min of X, max of X)
    //    - Expected HP change is -7.32153 (min of -21, max of +6)
    //    - Battle is lost 0 % of the time (expected remaining mob HP is X)
    //    Card stats :
    //    - Strike : played X / Y times (78%)
    //    - Defend : played X / Y times (27%)
    //    - Bash : played X / Y times (25.321%)
    //    - Ascender's Bane: unplayable, drawn 1 times
    //    Tree stats :
    //    - Expanded a total of X nodes.
    //    - Max of X nodes in use.
    //    - Completed tree contains X nodes including X terminal nodes.
    //    - Expected final objective of X
    //    - min of X
    //    - 1 % threshold of X
    //    - 5 % threshold of X
    //    - 25 % threshold of X
    //    - median of X
    //    - 75 % threshold of X
    //    - 95 % threshold of X
    //    - max of X
    void PrintTreeStats() {
        // debug
        for (auto & node_ptr : terminal_nodes) {
            if (!node_ptr->HasAncestor(*top_node_ptr)) {
                printf("ERROR: terminal node not a descendent of top node\n");
            }
        }

        printf("Memory stats:\n");
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
        // probability of ending up on each final HP
        std::map<int16_t, double> hp_delta;
        double expected_hp_delta = 0.0;
        std::map<uint16_t, double> turn;
        double expected_turn_count = 0.0;
        for (auto & node_ptr : terminal_nodes) {
            auto & p = node_ptr->probability;
            {
                int16_t x = (int) node_ptr->hp - top_node_ptr->hp;
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
        printf("Fight stats:\n");
        printf("- Expected HP change is %+g (min %+d, max %+d)\n",
            expected_hp_delta, hp_delta.begin()->first, hp_delta.rbegin()->first);
        printf("- Expected fight length is %g turns (min %u, max %u)\n",
            expected_turn_count, turn.begin()->first, turn.rbegin()->first);
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
            //if (expanded_node_count == 305) {
            //    node.PrintTree();
            //}
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
                    // TODO:
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
                // TODO: do we need to rethink this since sum of probabilities is no
                //       longer 1?
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
            //std::size_t max_solved_objective_index = 0;
            bool unsolved_children = false;
            double max_unsolved_objective = 0.0;
            for (std::size_t i = 0; i < node.child.size(); ++i) {
                Node & this_child = *node.child[i];
                if (this_child.tree_solved) {
                    if (!solved_children ||
                            this_child.composite_objective > max_solved_objective) {
                        max_solved_objective = this_child.composite_objective;
                        max_solved_objective_ptr = &this_child;
                        //max_solved_objective_index = i;
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
            if (node.tree_solved ||
                    max_unsolved_objective > node.composite_objective) {
                node.composite_objective = max_unsolved_objective;
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
            //assert(child[0]->probability == 1.0);
        }
        double p = 0.0;
        for (auto & ptr : node.child) {
            //if (player_choice) {
            //    assert(ptr->probability == 1.0);
            //}
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
        //if (!player_choice) {
        //    if (abs(p - 1.0) > 1e-10) {
        //        pass = false;
        //        printf("ERROR: probability (%g) != 1\n", p);
        //        PrintTree();
        //    }
        //}
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
        std::cout << "There are " <<
            top_node_ptr->deck.ptr->CountUniqueSubsets() <<
            " unique deck subsets\n";
        std::clock_t start_clock = clock();
        std::cout << "Expanding node: " << top_node_ptr->ToString() << "\n\n";
        if (normalize_mob_variations) {
            std::cout << "Mob variations in HP and stats are normalized.\n";
        }
        //assert(all_nodes.empty());
        //all_nodes.push_back(top_node_ptr);
        //critical_nodes.clear();
        optional_nodes.clear();
        //if (evaluate_critical_path) {
        //    critical_nodes.push_back(top_node_ptr);
        //} else {
        top_node_ptr->path_objective = top_node_ptr->GetPathObjective();
        optional_nodes.insert(top_node_ptr);
        //}
        expanded_node_count = 0;
        //node_choice_count = 0;
        //bool critical_path = evaluate_critical_path;
        std::clock_t next_update = clock();
        bool stats_shown = false;
        bool show_stats = true;

        std::size_t max_node_count = 0;

        //top_node_ptr->Verify(); // DEBUG

        // number of total nodes at which we prune the tree
        //std::size_t prune_cutoff = -1;
        // expand nodes until they're all done
        std::size_t iteration = 0;
        while (true) {
            ++iteration;
            // nodes in tree + deleted = created
            //top_node_ptr->PrintTree();
            //std::cout << ((int) created_node_count + 1 - top_node_ptr->CountNodes() + deleted_nodes.size()) << "\n";
            //printf("tree=%d, deleted=%d, created=%d\n",
            //    (int) top_node_ptr->CountNodes(),
            //    (int) deleted_nodes.size(),
            //    (int) created_node_count);
            //if (all_nodes.size() > max_node_count) {
            //    max_node_count = all_nodes.size();
            //}
            //top_node_ptr->PrintTree();
            //if (!VerifyNode(*top_node_ptr)) {
            //    printf("expanded_node_count=%u\n", (unsigned int) expanded_node_count);
            //    PrintTreeToFile("error_tree.txt");
            //    exit(0);
            //}
            // update every second
            stats_shown = false;
            if (show_stats || clock() >= next_update) {
                std::cout << "Tree stats: expanded=" << expanded_node_count <<
                    /*", all=" << all_nodes.size() <<*/
                    //", critical=" << critical_nodes.size() <<
                    ", optional=" << optional_nodes.size() << std::endl;
                PrintOptionalNodeProgress();
                    //printf("Tree stats: expanded=%u, all=%u, critical=%u, optional=%u\n",
                    //expanded_node_count,
                    //all_nodes.size(),
                    //critical_nodes.size(),
                    //optional_nodes.size());
                //printf("all=%u\n",
                //    top_node_ptr->CountNodes());
                //if (top_node_ptr->CountNodes() > 5000) {
                //    PrintTreeToFile("tree2.txt");
                //}
                next_update = clock() + CLOCKS_PER_SEC;
                stats_shown = true;
                show_stats = false;
            }
            // prune tree periodically
            //if (all_nodes.size() >= prune_cutoff) {
            //    // if we're still on the critical path, no nodes can be pruned
            //    //if (critical_path) {
            //    //    prune_cutoff = (std::size_t) (all_nodes.size() * 1.1);
            //    //    //printf("New pruning cutoff is %u\n", (unsigned int) prune_cutoff);
            //    //    continue;
            //    //}
            //    if (print_around_pruning) {
            //        show_stats = true;
            //    }
            //    if (print_around_pruning && !stats_shown) {
            //        continue;
            //    }
            //    // update max nodes present
            //    //if (all_nodes.size() > max_node_count) {
            //    //    max_node_count = all_nodes.size();
            //    //}
            //    Prune();
            //    if (prune_cutoff < all_nodes.size() * (double) 1.3) {
            //        prune_cutoff = (std::size_t) (all_nodes.size() * 1.3);
            //    }
            //    if (prune_cutoff > all_nodes.size() * (double) 1.5) {
            //        prune_cutoff = (std::size_t) (all_nodes.size() * 1.5);
            //    }
            //    continue;
            //}
            // if we're done, show stats and exit
            if (/*!critical_path && */optional_nodes.empty()) {
                //Prune();
                if (!stats_shown) {
                    show_stats = true;
                    continue;
                }
                break;
            }
            // find next node to expand and do it
            //bool is_critical = !critical_nodes.empty();
            //if (critical_path && !is_critical) {
            //    if (!stats_shown) {
            //        show_stats = true;
            //        continue;
            //    }
            //    critical_path = false;
            //    std::cout << "Expanded all critical nodes.  "
            //        << "All paths have an end.\n";
            //    printf("Tree stats: all=%u, total=%u, unsolved=%u\n",
            //        (unsigned int) all_nodes.size(),
            //        (unsigned int) top_node_ptr->CountNodes(),
            //        //(unsigned int) CountOrphanedNodes(),
            //        (unsigned int) top_node_ptr->CountUnsolvedLeaves());
            //    //std::cout << "Tree has " << CountOrphanedNodes() << " orphaned nodes\n";
            //    //std::cout << "Tree has " << top_node_ptr->CountNodes() << " total nodes\n";
            //    //std::cout << "Tree has " << top_node_ptr->CountUnsolvedLeaves() << " unsolved nodes\n";
            //    //top_node_ptr->PrintTree();
            //    std::cout << "Lower bound on final objective is " <<
            //        top_node_ptr->EstimateCompositeObjective() << ".\n";
            //    std::cout << "Upper bound on final objective is " <<
            //        top_node_ptr->composite_objective << ".\n";
            //    //if (all_nodes.size() < 100000) {
            //    //    PrintTreeToFile("critical_tree.txt");
            //    //}
            //    //top_node_ptr->Verify();
            //    //exit(0);
            //    //top_node_ptr->PrintTree();
            //    // The problem is when one choice the player can make is to die but it's (correctly)
            //    // deemed not the best choice.  This leads to two choices in the path which never
            //    // get cleaned up for some reason
            //    //top_node_ptr->Verify(); // DEBUG
            //    Prune();
            //    //top_node_ptr->Verify(); // DEBUG
            //    //std::cout << "Master tree has " <<
            //    //    top_node_ptr->CountUnsolvedLeaves() << " unsolved leaves\n";
            //    // restart loop in case no unsolved leaves remain
            //    if (print_around_pruning) {
            //        show_stats = true;
            //    }
            //    continue;
            //}
            //if (!is_critical) {
            //    //exit(0);
            //}
            Node * this_node_ptr = nullptr;
            //if (!critical_nodes.empty()) {
            //    this_node_ptr = *critical_nodes.begin();
            //    critical_nodes.pop_front();
            //} else {
            this_node_ptr = *optional_nodes.begin();
            optional_nodes.erase(optional_nodes.begin());
            //}
            //std::list<Node *> & this_list = (is_critical) ?
            //    critical_nodes : optional_nodes;
            //if (expanded_node_count == 26) {
            //    printf("");
            //}
            Node & this_node = *this_node_ptr;
            //printf("Expanding (%u): %s\n", (unsigned int) iteration, this_node.ToString().c_str());
            // see if this node is in the list
            if (/*!is_critical &&*/ !this_node.HasAncestor(*top_node_ptr)) {
                printf("ERROR: can we remove this?\n");
                //printf("Ignoring orphaned node\n");
                continue;
            }
            ++expanded_node_count;
            // if just starting, do start of battle initialization
            if (this_node.turn == 0) {
                this_node.player_choice = false;
                GenerateBattle(this_node);
                UpdateTree(&this_node);
                //VerifyNode(this_node);
                //VerifyNode(*top_node_ptr);
                //top_node_ptr->Verify();
                continue;
            }
            // if intents need generated, generate them
            if (this_node.generate_mob_intents) {
                //if (expanded_node_count == 28) {
                //    top_node_ptr->PrintTree(false, &this_node);
                //}
                GenerateMobIntents(this_node/*, is_critical*/);
                //if (expanded_node_count == 28) {
                //    top_node_ptr->PrintTree(false, &this_node);
                //}
                UpdateTree(&this_node);
                //if (expanded_node_count == 28) {
                //    top_node_ptr->PrintTree(false, &this_node);
                //}
                //VerifyNode(this_node);
                //VerifyNode(*top_node_ptr);
                //top_node_ptr->Verify();
                continue;
            }
            // if cards need drawn, draw them
            if (this_node.cards_to_draw) {
                DrawCards(this_node/*, is_critical*/);
                UpdateTree(&this_node);
                //VerifyNode(this_node);
                //VerifyNode(*top_node_ptr);
                //top_node_ptr->Verify();
                continue;
            }
            // else player can play a card or end turn
            //if (expanded_node_count == 109) {
            //    top_node_ptr->PrintTree(false, &this_node);
            //    this_node.PrintTree();
            //}
            //if (iteration == 24) {
            //    top_node_ptr->PrintTree(false, &this_node);
            //}
            FindPlayerChoices(this_node/*, is_critical*/);
            //if (iteration == 24) {
            //    top_node_ptr->PrintTree(false, &this_node);
            //}
            UpdateTree(&this_node);
            //if (iteration == 24) {
            //    top_node_ptr->PrintTree(false, &this_node);
            //    this_node.PrintTree();
            //    //this_node.CalculateCompositeObjectiveIncludingChildren();
            //    //this_node.PrintTree();
            //    printf("");
            //}
            //VerifyNode(*top_node_ptr);
            // node could have been discarded
            //if (this_node.HasAncestor(*top_node_ptr)) {
            //    VerifyNode(this_node);
            //}
        }
        // tree should now be solved
        const double duration = (double) (clock() - start_clock) / CLOCKS_PER_SEC;
        std::cout << "\n\n\n";
        std::cout << "Printing solved tree to tree.txt\n";
        std::cout << "Solution took " << duration << " seconds\n";
        top_node_ptr->PrintStats();
        std::cout << "Max nodes present was " << max_node_count << "\n";
        std::cout << "Deck map contains " <<
            CardCollectionMap::collection.size() << " decks\n";
        PrintTreeStats();
        //for (auto & it : CardCollectionMap::collection) {
        //    std::cout << it.ToString() << "\n";
        //}
        // print solved tree to file
        {
            std::ofstream outFile("tree.txt");
            std::streambuf * oldCoutStreamBuf = std::cout.rdbuf();
            std::cout.rdbuf(outFile.rdbuf());
            top_node_ptr->PrintStats();
            std::cout << "Max nodes present was " << max_node_count << "\n";
            std::cout << "Expanded " << expanded_node_count << " nodes\n";
            std::cout << "Solution took " << duration << " seconds.\n";
            std::cout << "Compiled on " << __DATE__ << " at " __TIME__ << "\n";
            std::cout << "\n";
            if (print_completed_tree_to_file) {
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

// entry point
int main(int argc, char ** argv) {
    std::cout << "sizeof(Node)=" << sizeof(Node) << std::endl;
    std::cout << "Can fit " << 1073741824 / sizeof(Node) << " nodes per GB\n";
    //printf("Command line arguments: %d\n", argc);
    //for (int i = 0; i < argc; ++i) {
    //    printf("- %s\n", argv[i]);
    //}

    //CardCollection deck;
    //std::cout << deck.ToString() << std::endl;
    //deck.AddCard(card_strike, 5);
    //deck.AddCard(card_defend, 4);
    //deck.AddCard(card_bash);
    //std::cout << deck.ToString() << std::endl;
    //for (auto pair : deck.Select(5)) {
    //    auto & selected = pair.second.first;
    //    std::cout << pair.first << ": " << selected.ToString() << std::endl;
    //}

    CardCollection ironclad_starting_deck;
    ironclad_starting_deck.AddCard(card_strike, 5);
    //ironclad_starting_deck.AddCard(card_strike_plus, 1);
    ironclad_starting_deck.AddCard(card_defend, 4);
    //ironclad_starting_deck.AddCard(card_defend_plus, 1);
    ironclad_starting_deck.AddCard(card_bash, 1);
    //ironclad_starting_deck.AddCard(card_bash_plus, 1);

    CardCollection cursed_ironclad_starting_deck = ironclad_starting_deck;
    cursed_ironclad_starting_deck.AddCard(card_ascenders_bane);


    //ironclad_starting_deck.AddCard(card_strike);

    // create top node
    Node top_node;
    top_node.deck = cursed_ironclad_starting_deck;
    //top_node.deck.AddCard(card_anger);
    //top_node.deck.AddCard(card_sword_boomerang);
    //top_node.deck.AddCard(card_inflame);
    //top_node.deck.AddCard(card_ascenders_bane);
    //top_node.deck.RemoveCard(card_strike.GetIndex());
    //top_node.fight_type = kFightAct1EasyCultist;
    //top_node.fight_type = kFightAct1EasyJawWorm;
    //top_node.fight_type = kFightAct1EasyLouses;
    //top_node.fight_type = kFightTestOneLouse;
    top_node.fight_type = kFightAct1EliteLagavulin;
    //top_node.fight_type = kFightAct1EliteGremlinNob;
    //top_node.fight_type = kFightAct1EasyJawWorm;
    top_node.max_hp = 75;
    top_node.hp = (uint16_t) (top_node.max_hp * 0.9);
    top_node.relics.burning_blood = 1;
    //top_node.relics.bronze_scales = 1;
    //top_node.relics.runic_pyramid = 1;

    top_node.InitializeStartingNode();

    TreeStruct tree(top_node);
    tree.Expand();

    //CompareRelics(top_node);
    exit(0);

}
