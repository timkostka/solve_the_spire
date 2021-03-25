#include <iostream>
#include <list>
#include <vector>
#include <deque>
#include <utility>
#include <ctime>
#include <sstream>
#include <fstream>

#include "card_collection.hpp"
#include "node.hpp"
#include "stopwatch.hpp"

/*

When we fill in a tree, we first try to find a path for all branches, then we
fill in the tree and select better choices when available.

When a choice comes back with multiple terminal nodes, we mark the one with the
highest to be expanded and put the others in reserve.  When all paths have a
solution, we go back to these reserve branches and expand them looking for
better options.

*/

// if true, will use average mob HP (saves much time)
const bool use_average_mob_hp = true;


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
    if (use_average_mob_hp) {
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
        layout.probability = 1.0;
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
    }
};

struct TreeStruct {
    // pointer to top node
    Node * top_node_ptr;
    // list of all created nodes
    std::list<Node *> all_nodes;
    // nodes which need expanded first
    // (since all must be evaluated, no need to sort them)
    std::list<Node *> critical_nodes;
    // nodes which need expanded after a path is formed
    std::set<Node *, PathObjectiveSort> optional_nodes;
    //std::list<Node *> optional_nodes;
    // number of nodes which were expanded
    uint32_t expanded_node_count;
    // number of terminal nodes found
    //uint32_t terminal_node_count;
    // number of nodes expanded with multiple choices
    // (this is not incremented if one choice is strictly better)
    uint32_t node_choice_count;
    // constructor
    TreeStruct(Node & node) : top_node_ptr(&node) {
        expanded_node_count = 0;
        node_choice_count = 0;
    }
    // add an optional node
    void AddOptionalNode(Node & node) {
        // should have already been evaluated
        assert(node.path_objective == node.GetPathObjective());
        // add it
        optional_nodes.insert(&node);
    }
    // create a new node and return the pointer
    Node & CreateChild(Node & node, bool add_to_critical, bool add_to_optional) {
        assert(!(add_to_critical && add_to_optional));
        Node * new_node_ptr = new Node(node);
        if (new_node_ptr == nullptr) {
            printf("ERROR: out of memory\n");
            exit(1);
        }
        Node & new_node = *new_node_ptr;
        new_node.child.clear();
        new_node.parent = &node;
        all_nodes.push_back(&new_node);
        node.child.push_back(&new_node);
        if (add_to_critical) {
            critical_nodes.push_back(&new_node);
        } else if (add_to_optional) {
            AddOptionalNode(new_node);
            //optional_nodes.push_back(&new_node);
        }
        return new_node;
    }
    // prune orphaned nodes from the tree
    void Prune() {
        Node & top_node = *top_node_ptr;
        Stopwatch watch;
        // prune optional_nodes
        //std::size_t prune_count_1 = 0;
        {
            auto it = optional_nodes.begin();
            while (it != optional_nodes.end()) {
                Node & node = **it;
                if (!node.HasAncestor(top_node)) {
                    it = optional_nodes.erase(it);
                    //++prune_count_1;
                } else {
                    ++it;
                }
            }
        }
        // prune all_nodes
        // go in reverse order to ensure children are erased before parents
        std::size_t prune_count = 0;
        {
            auto it = all_nodes.end();
            while (it != all_nodes.begin()) {
                --it;
                Node & node = **it;
                if (!node.HasAncestor(top_node)) {
                    delete &node;
                    auto it2 = it;
                    it = all_nodes.erase(it);
                    ++prune_count;
                }
            }
        }
        printf("Pruned %.3g%% (%u/%u) of nodes in %.3g seconds\n",
            100.0 * prune_count / (all_nodes.size() + prune_count),
            (unsigned int) prune_count,
            (unsigned int) (all_nodes.size() + prune_count),
            watch.GetTime());
        //printf("Pruned %u (%.3g%%) nodes from optional_nodes\n",
        //    prune_count_1,
        //    100.0 * prune_count_1 / (optional_nodes.size() + prune_count_1));
        //printf("Pruned %u (%.3g%%) nodes from all_nodes\n",
        //    prune_count_2,
        //    100.0 * prune_count_2 / (all_nodes.size() + prune_count_2));
        //printf("Pruning took %.3g seconds\n", watch.GetTime());
    }
    // generate mob intents
    void GenerateMobIntents(Node & node, bool is_critical) {
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
            Node & new_node = CreateChild(node, is_critical, !is_critical);
            new_node.generate_mob_intents = false;
            new_node.probability = 1.0;
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
    void DrawCards(Node & node, bool is_critical) {
        assert(node.cards_to_draw > 0);
        node.player_choice = false;
        // if draw pile is empty, move cards from discard to draw pile
        if (node.draw_pile.IsEmpty() && !node.discard_pile.IsEmpty()) {
            // add new node
            Node & new_node = CreateChild(node, is_critical, !is_critical);
            new_node.probability = 1.0;
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
            Node & new_node = CreateChild(node, is_critical, !is_critical);
            // we have no cards to draw and/or our hand is full
            new_node.cards_to_draw = 0;
            return;
        }
        // else draw all cards we can and add each as a child node
        auto choices = node.draw_pile.Select(to_draw);
        for (const auto & choice : choices) {
            // add new node
            Node & new_node = CreateChild(node, is_critical, !is_critical);
            new_node.cards_to_draw -= to_draw;
            new_node.probability = choice.first;
            new_node.hand.AddCard(choice.second.first);
            new_node.draw_pile = choice.second.second;
            new_node.child.clear();
        }
    }
    // change the tree such that the top node only makes choices which end up
    // at the path node
    // update composite_object/tree_solved of nodes below top node
    void SelectTerminalDecisionPath(Node & top_node, Node & path_node) {
        assert(path_node.IsBattleDone());
        assert(path_node.tree_solved);
        assert(path_node.child.empty());
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
                parent.child[0] = &node;
                parent.child.resize(1);
            } else {
                assert(parent.child[0] == node_ptr);
            }
            // update objectives
            parent.composite_objective = node.composite_objective;
            assert(node.tree_solved);
            parent.tree_solved = true;
            node_ptr = &parent;
        }
    }
    // add node pointers to set
    // (helper function used during FindPlayerChoices)
    void AddNodesToSet(Node & node, std::set<Node *> & node_set) {
        node_set.insert(&node);
        for (auto this_child : node.child) {
            node_set.insert(this_child);
            AddNodesToSet(*this_child, node_set);
        }
    }
    // delete nodes which are no longer in tree
    // (helper function used during FindPlayerChoices)
    void DeleteUnusedPlayerChoices(
            Node & top_node, Node * const last_stored_node_ptr) {
        std::size_t deleted_count = 0;
        std::size_t kept_count = 0;
        // store used nodes into set
        std::set<Node *> used_node;
        AddNodesToSet(top_node, used_node);
        // go through nodes and delete unused ones
        auto it = all_nodes.end();
        while (*(--it) != last_stored_node_ptr) {
            Node * node_ptr = *it;
            if (used_node.find(node_ptr) == used_node.end()) {
                delete node_ptr;
                it = all_nodes.erase(it);
            }
        }
    }
    // 
    // find player choice nodes
    void FindPlayerChoices(Node & top_node, bool is_critical) {
        top_node.player_choice = true;
        // hold pointer to last item in all_nodes
        Node * const last_stored_node_ptr = *all_nodes.rbegin();
        //std::cout << "\nExpanding: " << ToString() << "\n";
        // nodes we must make a decision at
        std::vector<Node *> decision_nodes;
        decision_nodes.push_back(&top_node);
        // nodes at which the player no longer has a choice
        // (e.g. after pressing end turn or after player or all mobs are dead)
        std::vector<Node *> terminal_node;
        //bool dead_node = false;
        // expand all decision nodes
        while (!decision_nodes.empty()) {
            // loop through each node we need to expand
            std::vector<Node *> new_decision_nodes;
            for (auto & this_node_ptr : decision_nodes) {
                Node & this_node = *this_node_ptr;
                // add end the turn node
                Node & new_node = CreateChild(this_node, false, false);
                new_node.probability = 1.0;
                new_node.EndTurn();
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
                    DeleteUnusedPlayerChoices(top_node, last_stored_node_ptr);
                    //top_node.UpdateParents();
                    return;
                }
                terminal_node.push_back(&new_node);
                // play all possible unique cards
                for (std::size_t i = 0; i < this_node.hand.card.size(); ++i) {
                    // alias the card
                    auto & card = *card_index[this_node.hand.card[i].first];
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
                            Node & new_node = CreateChild(this_node, false, false);
                            new_node.probability = 1.0;
                            uint16_t index = this_node.hand.card[i].first;
                            new_node.hand.RemoveCard(index);
                            new_node.PlayCard(index, m);
                            // if this is the best possible objective,
                            // don't process any further choices
                            if (new_node.IsBattleDone() &&
                                    new_node.GetMaxFinalObjective() ==
                                    top_node.GetMaxFinalObjective()) {
                                SelectTerminalDecisionPath(top_node, new_node);
                                DeleteUnusedPlayerChoices(
                                    top_node, last_stored_node_ptr);
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
                                terminal_node.push_back(&new_node);
                            } else {
                                new_decision_nodes.push_back(&new_node);
                            }
                        }
                    } else {
                        Node & new_node = CreateChild(this_node, false, false);
                        new_node.probability = 1.0;
                        uint16_t index = this_node.hand.card[i].first;
                        new_node.hand.RemoveCard(index);
                        new_node.PlayCard(index);
                        // if this is the best possible objective,
                        // don't process any further choices
                        if (new_node.IsBattleDone() &&
                                new_node.GetMaxFinalObjective() ==
                                top_node.GetMaxFinalObjective()) {
                            SelectTerminalDecisionPath(top_node, new_node);
                            DeleteUnusedPlayerChoices(
                                top_node, last_stored_node_ptr);
                            //top_node.UpdateParents();
                            return;
                        }
                        if (card.Exhausts()) {
                            new_node.exhaust_pile.AddCard(index);
                        } else {
                            new_node.discard_pile.AddCard(index);
                        }
                        if (new_node.IsBattleDone()) {
                            terminal_node.push_back(&new_node);
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
        std::vector<bool> bad_node(terminal_node.size(), false);
        for (std::size_t i = 0; i < terminal_node.size(); ++i) {
            if (bad_node[i]) {
                continue;
            }
            Node & node_i = *terminal_node[i];
            for (std::size_t j = 0; j < terminal_node.size(); ++j) {
                if (i == j || bad_node[j]) {
                    continue;
                }
                Node & node_j = *terminal_node[j];
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
        for (std::size_t i = 0; i < terminal_node.size(); ++i) {
            Node & node_i = *terminal_node[i];
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
            for (std::size_t i = 0; i < terminal_node.size(); ++i) {
                Node & node_i = *terminal_node[i];
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
        for (std::size_t i = 0; i < terminal_node.size(); ++i) {
            if (!bad_node[i]) {
                continue;
            }
            // remove this node from its parent and prune empty nodes
            Node * node_ptr = terminal_node[i];
            while (node_ptr != &top_node && node_ptr->child.empty()) {
                auto & vec = node_ptr->parent->child;
                auto it = std::find(vec.begin(), vec.end(), node_ptr);
                assert(it != vec.end());
                vec.erase(it);
                node_ptr = node_ptr->parent;
            }
        }
        // calculate composite objective of all nodes still in tree
        // and delete unused nodes
        {
            auto it = all_nodes.end();
            while (*(--it) != last_stored_node_ptr) {
                Node & node = **it;
                // if node is no longer in the tree, delete it
                if (!node.HasAncestor(top_node)) {
                    delete &node;
                    it = all_nodes.erase(it);
                    continue;
                }
                // at this point, if node has no children, it should be listed
                // as a good node (i.e. not listed as a bad node)
                // calculate objective
                node.composite_objective = node.CalculateCompositeObjective();
                // propagate solved tree state
                if (node.child.size() == 1 && node.child[0]->tree_solved) {
                    node.tree_solved = true;
                }
            }
        }
        // if only one terminal node is left, and it ends the battle, update parents
        // (this is usually triggered if all choices lead to death)
        //assert(good_node_count > 0);
        //if (good_node_count == 1) {
        //    if (good_terminal_node_ptr->tree_solved) {
        //        //top_node.PrintTree();
        //        //top_node_ptr->PrintTree();
        //        //printf("");
        //        good_terminal_node_ptr->UpdateParents();
        //        //top_node_ptr->PrintTree();
        //        return;
        //    }
        //}
        // if we're on a critical path and end of battle was found on a good
        // node, all paths which don't end the fight are optional
        if (is_critical) {
            for (std::size_t i = 0; i < terminal_node.size(); ++i) {
                if (bad_node[i]) {
                    continue;
                }
                if (terminal_node[i]->IsBattleDone()) {
                    is_critical = false;
                    break;
                }
            }
        }
        // if not on a critical path, all decisions are optional
        if (!is_critical) {
            for (std::size_t i = 0; i < terminal_node.size(); ++i) {
                if (bad_node[i]) {
                    continue;
                }
                Node & this_node = *terminal_node[i];
                this_node.path_objective = this_node.GetPathObjective();
                if (!this_node.IsBattleDone()) {
                    //optional_nodes.push_back(&this_node);
                    AddOptionalNode(this_node);
                }
            }
            return;
        }
        // if we're on a critical path, select choice with best path objective
        // to add to critical path nodes and add others to optional nodes
        double best_objective = 0.0;
        std::size_t best_objective_index = -1;
        for (std::size_t i = 0; i < terminal_node.size(); ++i) {
            if (bad_node[i]) {
                continue;
            }
            assert(!terminal_node[i]->IsBattleDone());
            double objective = terminal_node[i]->GetPathObjective();
            terminal_node[i]->path_objective = objective;
            if (best_objective_index == -1) {
                best_objective = objective;
                best_objective_index = i;
            } else if (objective > best_objective) {
                //optional_nodes.push_back(terminal_node[best_objective_index]);
                AddOptionalNode(*terminal_node[best_objective_index]);
                best_objective = objective;
                best_objective_index = i;
            } else {
                //optional_nodes.push_back(terminal_node[i]);
                AddOptionalNode(*terminal_node[i]);
            }
        }
        assert(best_objective_index != -1);
        critical_nodes.push_back(terminal_node[best_objective_index]);
    }
    // return total number of orphaned nodes
    std::size_t CountOrphanedNodes() {
        std::size_t result = 0;
        for (auto & it : all_nodes) {
            if (!it->HasAncestor(*top_node_ptr)) {
                ++result;
            }
        }
        return result;
    }
    // start new battle and generate mobs
    void GenerateBattle(Node & this_node) {
        assert(this_node.turn == 0);
        for (auto & layout : GenerateAllMobs(this_node.fight_type)) {
            // create one node per mob layout
            Node & new_node = CreateChild(this_node, true, false);
            new_node.StartBattle();
            new_node.probability = layout.probability;
            for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
                new_node.monster[i] = layout.mob[i];
            }
            new_node.composite_objective = new_node.GetMaxFinalObjective();
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
    // expand this tree
    void Expand() {
        std::clock_t start_clock = clock();
        std::cout << "Expanding node: " << top_node_ptr->ToString() << "\n\n";
        assert(all_nodes.empty());
        all_nodes.push_back(top_node_ptr);
        critical_nodes.clear();
        critical_nodes.push_back(top_node_ptr);
        optional_nodes.clear();
        expanded_node_count = 0;
        //terminal_node_count = 0;
        node_choice_count = 0;
        bool critical_path = true;
        std::clock_t next_update = clock();
        bool stats_shown = false;
        bool show_stats = true;

        std::size_t max_node_count = 0;

        //top_node_ptr->Verify(); // DEBUG

        // number of total nodes at which we prune the tree
        std::size_t prune_cutoff = 1000;
        // expand nodes until they're all done
        while (true) {
            //if (!top_node_ptr->Verify()) {
            //    PrintTreeToFile("error_tree.txt");
            //    exit(0);
            //}
            // update every second
            stats_shown = false;
            if (show_stats || clock() >= next_update) {
                std::cout << "Tree stats: expanded=" << expanded_node_count <<
                    ", all=" << all_nodes.size() <<
                    ", critical=" << critical_nodes.size() <<
                    ", optional=" << optional_nodes.size() << std::endl;
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
            if (all_nodes.size() >= prune_cutoff) {
                // if we're still on the critical path, no nodes can be pruned
                if (critical_path) {
                    prune_cutoff = (std::size_t) (all_nodes.size() * 1.1);
                    //printf("New pruning cutoff is %u\n", (unsigned int) prune_cutoff);
                    continue;
                }
                show_stats = true;
                if (!stats_shown) {
                    continue;
                }
                // update max nodes present
                if (all_nodes.size() > max_node_count) {
                    max_node_count = all_nodes.size();
                }
                Prune();
                //prune_cutoff = (std::size_t) (all_nodes.size() * 1.5);
                if (all_nodes.size() * (double) 1.5 > prune_cutoff) {
                    prune_cutoff = (std::size_t) (all_nodes.size() * 1.5);
                    //printf("New pruning cutoff is %u\n", (unsigned int) prune_cutoff);
                }
                continue;
            }
            // if we're done, show stats and exit
            if (!critical_path && optional_nodes.empty()) {
                Prune();
                if (!stats_shown) {
                    show_stats = true;
                    continue;
                }
                break;
            }
            // find next node to expand and do it
            bool is_critical = !critical_nodes.empty();
            if (critical_path && !is_critical) {
                if (!stats_shown) {
                    show_stats = true;
                    continue;
                }
                critical_path = false;
                std::cout << "Expanded all critical nodes.  "
                    << "All paths have an end.\n";
                printf("Tree stats: all=%u, total=%u, unsolved=%u\n",
                    (unsigned int) all_nodes.size(),
                    (unsigned int) top_node_ptr->CountNodes(),
                    //(unsigned int) CountOrphanedNodes(),
                    (unsigned int) top_node_ptr->CountUnsolvedLeaves());
                //std::cout << "Tree has " << CountOrphanedNodes() << " orphaned nodes\n";
                //std::cout << "Tree has " << top_node_ptr->CountNodes() << " total nodes\n";
                //std::cout << "Tree has " << top_node_ptr->CountUnsolvedLeaves() << " unsolved nodes\n";
                //top_node_ptr->PrintTree();
                std::cout << "Lower bound on final objective is " <<
                    top_node_ptr->EstimateCompositeObjective() << ".\n";
                std::cout << "Upper bound on final objective is " <<
                    top_node_ptr->composite_objective << ".\n";
                if (all_nodes.size() < 100000) {
                    PrintTreeToFile("critical_tree.txt");
                }
                //top_node_ptr->Verify();
                //exit(0);
                //top_node_ptr->PrintTree();
                // The problem is when one choice the player can make is to die but it's (correctly)
                // deemed not the best choice.  This leads to two choices in the path which never
                // get cleaned up for some reason
                //top_node_ptr->Verify(); // DEBUG
                Prune();
                //top_node_ptr->Verify(); // DEBUG
                //std::cout << "Master tree has " <<
                //    top_node_ptr->CountUnsolvedLeaves() << " unsolved leaves\n";
                // restart loop in case no unsolved leaves remain
                show_stats = true;
                continue;
            }
            if (!is_critical) {
                //exit(0);
            }
            Node * this_node_ptr = nullptr;
            if (!critical_nodes.empty()) {
                this_node_ptr = *critical_nodes.begin();
                critical_nodes.pop_front();
            } else {
                this_node_ptr = *optional_nodes.begin();
                optional_nodes.erase(optional_nodes.begin());
            }
            //std::list<Node *> & this_list = (is_critical) ?
            //    critical_nodes : optional_nodes;
            Node & this_node = *this_node_ptr;
            // see if this node is in the list
            if (!is_critical && !this_node.HasAncestor(*top_node_ptr)) {
                //printf("Ignoring orphaned node\n");
                continue;
            }
            ++expanded_node_count;
            // if just starting, do start of battle initialization
            if (this_node.turn == 0) {
                this_node.player_choice = false;
                GenerateBattle(this_node);
                this_node.UpdateTree();
                //top_node_ptr->Verify();
                continue;
            }
            // if intents need generated, generate them
            if (this_node.generate_mob_intents) {
                GenerateMobIntents(this_node, is_critical);
                this_node.UpdateTree();
                //top_node_ptr->Verify();
                continue;
            }
            // if cards need drawn, draw them
            if (this_node.cards_to_draw) {
                DrawCards(this_node, is_critical);
                this_node.UpdateTree();
                //top_node_ptr->Verify();
                continue;
            }
            // else player can play a card or end turn
            FindPlayerChoices(this_node, is_critical);
            this_node.UpdateTree();
            //top_node_ptr->Verify();
        }
        // tree should now be solved
        const double duration = (double) (clock() - start_clock) / CLOCKS_PER_SEC;
        std::cout << "\n\n\n";
        std::cout << "Printing solved tree to tree.txt\n";
        std::cout << "Solution took " << duration << " seconds\n";
        top_node_ptr->PrintStats();
        std::cout << "Max nodes present was " << max_node_count << "\n";
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
            top_node_ptr->PrintTree();
            std::cout.rdbuf(oldCoutStreamBuf);
            outFile.close();
        }
        top_node_ptr->Verify();
    }
};

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

    //ironclad_starting_deck.AddCard(card_strike);

    // create top node
    Node top_node;
    top_node.deck = ironclad_starting_deck;
    //top_node.deck.AddCard(card_flex);
    //top_node.deck.AddCard(card_sword_boomerang);
    //top_node.deck.AddCard(card_inflame);
    //top_node.deck.RemoveCard(card_strike.GetIndex());
    //top_node.fight_type = kFightAct1EasyCultist;
    //top_node.fight_type = kFightAct1EasyJawWorm;
    top_node.fight_type = kFightAct1EliteLagavulin;
    //top_node.fight_type = kFightAct1EliteGremlinNob;
    //top_node.fight_type = kFightAct1EasyJawWorm;
    top_node.max_hp = 75;
    top_node.hp = (uint16_t) (top_node.max_hp * 0.9);

    top_node.InitializeStartingNode();

    TreeStruct tree(top_node);
    tree.Expand();

}
