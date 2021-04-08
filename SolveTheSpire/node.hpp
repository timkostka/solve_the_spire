#pragma once

#include <cstdint>
#include <vector>
#include <sstream>
#include <set>
#include <list>
#include <deque>
#include <algorithm>
#include <map>
#include <iostream>

#include "defines.h"
#include "card_collection.hpp"
#include "card_collection_map.hpp"
#include "card.hpp"
#include "buff_state.hpp"
#include "monster.hpp"
#include "relics.hpp"
#include "fight.hpp"

// map of decks
// Each node stores a pointer to a deck within this collection rather than
// storing the entire collection.  This reduces the number of std::vectors
// within the Node along with the associated create/copy/delete operations.
//CardCollectionMapStruct card_collections;

// enum for a decision
enum DecisionTypeEnum : uint8_t {
    kDecisionUnused = 0,
    // play a card
    kDecisionPlayCard,
    // end the turn
    kDecisionEndTurn,
};

// decision made by the player
struct Decision {
    // decision type
    DecisionTypeEnum type;
    // mob number to target or 0
    uint16_t argument[2];
    // convert to a string
    std::string ToString() const {
        std::ostringstream ss;
        if (type == kDecisionPlayCard) {
            const Card & card = *card_index[argument[0]];
            ss << "play " << card.name;
        } else if (type == kDecisionEndTurn) {
            return "end turn";
        } else {
            ss << "unknown";
        }
        return ss.str();
    }
};

// A Node contains all information about a game node.
struct Node {
    // fight type
    FightEnum fight_type;
    // turn number
    uint16_t turn;
    // max player HP
    uint16_t max_hp;
    // relic state
    RelicStruct relics;
    // current player HP
    uint16_t hp;
    // amount of block
    uint16_t block;
    // pointer to deck
    CardCollectionPtr deck;
    // hand
    CardCollectionPtr hand;
    // draw pile
    CardCollectionPtr draw_pile;
    // discard pile
    CardCollectionPtr discard_pile;
    // exhausted pile
    CardCollectionPtr exhaust_pile;
    // true if new mob intents need generated
    bool generate_mob_intents;
    // monsters (in order of action)
    Monster monster[MAX_MOBS_PER_NODE];
    // player energy
    uint16_t energy;
    // buffs
    BuffState buff;
    // link to parent node
    Node * parent;
    // list of children
    vector<Node *> child;
    // true if the next move is to play a card or end turn
    bool player_choice;
    // probability of getting to this node from the parent
    // (only valid if player_choice of parent is false)
    double probability;
    // decision at parent node in order to get to this node
    // (only valid if player_choice of parent is false)
    Decision parent_decision;
    // max final hp
    //uint16_t max_final_hp;
    // expected final hp
    //double expected_final_hp;
    // number of cards left to draw for this turn
    uint16_t cards_to_draw;
    // true if battle is complete
    // (battle is complete if all mobs are dead or if player is dead)
    bool battle_done;
    // true if tree below this is solved
    bool tree_solved;
    // maximum possible composite objective
    // (if tree_solved=true, this is the final composite objective)
    double composite_objective;
    // path objective (used to sort optional nodes for evaluating)
    double path_objective;
    // default constructor
    //Node() :
    //    fight_type(kFightAct1EasyCultist),
    //    turn(0),
    //    max_hp(0),
    //    hp(0),
    //    block(0),
    //    deck(),
    //    hand(),
    //    draw_pile(),
    //    tree_solved(false),
    //    battle_done(false),
    //    cards_to_draw(0),
    //    composite_objective(0.0),
    //    energy(0) {
    //    // TODO
    //}
    // add a single child node with 100% probability
    void AddChild(Node & child_node) {
        child.push_back(&child_node);
        child_node.parent = this;
    }
    // return true if player is dead
    bool IsDead() const {
        return hp == 0;
    }
    // return true if battle is complete
    bool IsBattleDone() const {
        return battle_done;
    }
    // return an objective function used to evaluate the best decision
    // this function may only use information within this node--no information
    // from children is allowed
    double GetPathObjective() const {
        double x = 5.0 * hp;
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            if (monster[i].Exists()) {
                x -= monster[i].hp;
            }
        }
        // favor evaluating later turns
        x += turn * 1000;
        return x;
    }
    // return best possible ultimate objective function for a child of this node
    double GetMaxFinalObjective() const {
        if (IsBattleDone()) {
            return hp;
        }
        uint16_t top = hp;
        if (relics.burning_blood) {
            top += 6;
        }
        if (top > max_hp) {
            top = max_hp;
        }
        return top;
    }
    // return the final objective of an end-node
    // (may only be called on terminal nodes)
    void CalculateFinalObjective() {
        assert(IsBattleDone());
        assert(child.empty());
        tree_solved = true;
        composite_objective = hp;
    }
    // calculate composite objective for a tree with multiple paths
    //void CalculateCompositeObjective() {
    //    assert(child.size() >= 1);
    //    assert(!player_choice);
    //    double max_objective = child[0]->composite_objective;
    //    double min_objective = max_objective;
    //    composite_objective = 0.0;
    //    for (auto child_ptr : child) {
    //        assert(child_ptr->tree_solved);
    //        composite_objective +=
    //            child_ptr->probability * child_ptr->composite_objective;
    //        if (child_ptr->composite_objective > max_objective) {
    //            max_objective = child_ptr->composite_objective;
    //        }
    //        if (child_ptr->composite_objective < min_objective) {
    //            min_objective = child_ptr->composite_objective;
    //        }
    //    }
    //    // in order to avoid roundoff errors causing a different value if all
    //    // children have the same objective, we check this manually
    //    if (min_objective == max_objective) {
    //        composite_objective = min_objective;
    //    }
    //    tree_solved = true;
    //}
    // initialize a start of fight node
    // (only things that need initialized outside of this are:
    // hp, max_hp, fight_type, deck
    void InitializeStartingNode() {
        assert(hp > 0);
        assert(max_hp >= hp);
        assert(!deck.IsEmpty());

        turn = 0;
        block = 0;
        probability = 1.0;
        energy = 0;
        cards_to_draw = 0;
        generate_mob_intents = true;
        player_choice = false;
        parent = nullptr;
        buff.Reset();
        tree_solved = false;
        composite_objective = GetMaxFinalObjective();
        path_objective = GetPathObjective();
    }
    // estimate composite objective for a tree with at least one solved child
    double EstimateCompositeObjective() {
        // get a list of final states and probability of reaching each one
        std::list<std::pair<double, Node *>> end_states;
        FindTerminalLeaves(1.0, end_states);
        double check = 0.0;
        for (auto & ptr : end_states) {
            check += ptr.first;
        }
        if (abs(check - 1.0) > 1e-10) {
            printf("ERROR: total probability (%g) not 1\n", check);
        }
        double result = 0.0;
        for (auto & item : end_states) {
            result += item.first * item.second->composite_objective;
        }
        return result;
    }
    // return total number of nodes including this one and below it
    std::size_t CountNodes() {
        std::size_t count = 1;
        for (auto & child_ptr : child) {
            count += child_ptr->CountNodes();
        }
        return count;
    }
    // return the number of unsolved nodes without children
    std::size_t CountUnsolvedLeaves() {
        if (tree_solved) {
            return 0;
        }
        if (child.empty()) {
            return (tree_solved) ? 0 : 1;
        }
        std::size_t count = 0;
        for (auto & child_ptr : child) {
            count += child_ptr->CountUnsolvedLeaves();
        }
        return count;
    }
    // return true if this node is an orphan
    bool IsOrphan() const {
        for (auto & child_ptr : parent->child) {
            if (child_ptr == this) {
                return false;
            }
        }
        return true;
    }
    // return true if this node has the given ancestor
    bool HasAncestor(const Node & that) const {
        const Node * node_ptr = this;
        while (node_ptr != &that) {
            if (node_ptr->parent == nullptr) {
                return false;
            }
            if (node_ptr->IsOrphan()) {
                return false;
            }
            node_ptr = node_ptr->parent;
        }
        return true;
    }
    // try to solve this tree and return true if successful
    // (may only be called on nodes with one or more solved children)
    bool SolveTree() {
        assert(!tree_solved);
        // if terminal node, objective is easy
        assert(!child.empty());
        if (child.empty()) {
            assert(tree_solved);
            CalculateFinalObjective();
            tree_solved = true;
        } else if (child.size() == 1) {
            // pretty sure we always are in this case
            assert(child[0]->tree_solved);
            composite_objective = child[0]->composite_objective;
            tree_solved = true;
        } else {
            // if not a player choice, calculate composite from children
            if (!player_choice) {
                // if this is not a player choice, all children need to be solved
                for (auto & this_child : child) {
                    if (!this_child->tree_solved) {
                        return false;
                    }
                }
                composite_objective = CalculateCompositeObjective();
                tree_solved = true;
            }
            // if it is a player choice, make the best decision
            if (player_choice) {
                bool found_solved = false;
                // find best objective out of solved children
                double best_solved_objective = 0.0;
                //
                bool found_unsolved = false;
                // index of best solved objective
                std::size_t best_solved_objective_index = 0;
                // out of unsolved children, hold max bound on potential composite
                double max_unsolved_objective = 0.0;
                for (std::size_t i = 0; i < child.size(); ++i) {
                    if (child[i]->tree_solved) {
                        double & x = child[i]->composite_objective;
                        if (!found_solved || x > best_solved_objective) {
                            best_solved_objective = x;
                            best_solved_objective_index = i;
                            found_solved = true;
                        }
                    } else {
                        double x = child[i]->GetMaxFinalObjective();
                        if (!found_unsolved || x > max_unsolved_objective) {
                            max_unsolved_objective = x;
                            found_unsolved = true;
                        }
                    }
                }
                // if unsolved children can be better, we can't do anything
                if (found_unsolved &&
                        max_unsolved_objective > best_solved_objective) {
                    //std::cout << "Potential improvement found\n";
                    //PrintTree();
                    return false;
                }
                if (found_unsolved) {
                    //std::cout << "Orphaned unsolved nodes\n";
                    //PrintTree();
                }
                // select best choice
                // (this is the point at which we can orphan nodes)
                child[0] = child[best_solved_objective_index];
                child.resize(1);
                tree_solved = true;
                composite_objective = best_solved_objective;
                if (found_unsolved) {
                    //PrintTree();
                    //printf("");
                }
            }
        }
        return true;
    }
    // update parents to prune bad choice and/or mark trees as solved
    //void UpdateParents() {
    //    // should only be called if this tree is solved
    //    assert(tree_solved);
    //    Node * node_ptr = this;
    //    while (node_ptr->parent != nullptr) {
    //        Node & parent = *node_ptr->parent;
    //        // if only one child, easy enough to calculate
    //        if (!parent.SolveTree()) {
    //            break;
    //        }
    //        node_ptr = &parent;
    //    }
    //}
    // return an estimate for the composite objective by looking at direct children
    double CalculateCompositeObjective() {
        // if no children, just get max
        if (child.empty()) {
            return GetMaxFinalObjective();
        }
        // if one child, just return objective from that child
        if (child.size() == 1) {
            return child[0]->composite_objective;
        }
        // if children and players choice, return max
        if (player_choice) {
            double max_objective = child[0]->composite_objective;
            //std::size_t max_objective_index = 0;
            for (std::size_t i = 1; i < child.size(); ++i) {
                if (child[i]->composite_objective > max_objective) {
                    max_objective = child[i]->composite_objective;
                    //max_objective_index = i;
                }
            }
            return max_objective;
        } else {
            // if not player choice, then return probability adjusted composite
            double objective = child[0]->probability * child[0]->composite_objective;
            double max_objective = child[0]->composite_objective;
            double min_objective = max_objective;
            for (std::size_t i = 1; i < child.size(); ++i) {
                objective += child[i]->probability * child[i]->composite_objective;
                if (child[i]->composite_objective > max_objective) {
                    max_objective = child[i]->composite_objective;
                    //max_objective_index = i;
                } else if (child[i]->composite_objective < min_objective) {
                    min_objective = child[i]->composite_objective;
                    //max_objective_index = i;
                }
            }
            // to prevent roundoff errors from propagating
            if (max_objective == min_objective) {
                objective = max_objective;
            }
            return objective;
        }
    }
    // return true if all children are solved
    bool AreChildrenSolved() {
        assert(!child.empty());
        for (auto & it : child) {
            if (!it->tree_solved) {
                return false;
            }
        }
        return true;
    }
    // update this node.  if we find changes, update parents until we no
    // longer find changes.  This is only ever called on nodes with children.
    void UpdateTree() {
        // doesn't make sense to call this on a solved node
        assert(!tree_solved);
        // if terminal node, objective is easy
        assert(!child.empty());
        // if only one child, objective is the same as that child
        if (child.size() == 1) {
            // if it's different, update and update parent
            if (composite_objective != child[0]->composite_objective ||
                   tree_solved != child[0]->tree_solved) {
                composite_objective = child[0]->composite_objective;
                tree_solved = child[0]->tree_solved;
                if (parent != nullptr) {
                    parent->UpdateTree();
                }
            }
            return;
        }
        assert(child.size() > 1);
        // if not player choice, objective is probability weighted average of
        // children and tree is solved iff all children are solved
        if (!player_choice) {
            double x = CalculateCompositeObjective();
            bool solved = AreChildrenSolved();
            if (composite_objective != x || tree_solved != solved) {
                composite_objective = x;
                tree_solved = solved;
                if (parent != nullptr) {
                    parent->UpdateTree();
                }
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
        std::size_t max_solved_objective_index = 0;
        bool unsolved_children = false;
        double max_unsolved_objective = 0.0;
        for (std::size_t i = 0; i < child.size(); ++i) {
            const Node & this_child = *child[i];
            if (this_child.tree_solved) {
                if (!solved_children ||
                    this_child.composite_objective > max_solved_objective) {
                    max_solved_objective = this_child.composite_objective;
                    max_solved_objective_index = i;
                }
                solved_children = true;
            } else {
                unsolved_children = true;
                max_unsolved_objective =
                    std::max(max_unsolved_objective,
                        this_child.composite_objective);
            }
        }
        // (1) if all children solved, choose the best one
        if (!unsolved_children) {
            // delete other children
            child[0] = child[max_solved_objective_index];
            child.resize(1);
            assert(!tree_solved);
            tree_solved = true;
            assert(child[0]->composite_objective == max_solved_objective);
            composite_objective = max_solved_objective;
            if (parent != nullptr) {
                parent->UpdateTree();
            }
            return;
        }
        // (2) if no children are solved, objective is the highest child objective
        if (!solved_children) {
            if (composite_objective != max_unsolved_objective) {
                    composite_objective = max_unsolved_objective;
                if (parent != nullptr) {
                    parent->UpdateTree();
                }
            }
            return;
        }
        // (3) if some children are solved, eliminate unsolved paths with
        //     max objectives below the max solved objective and also
        //     eliminate solved paths that are not optimal
        assert(solved_children && unsolved_children);
        {
            std::size_t i = child.size();
            while (i > 0) {
                assert(i > 0);
                --i;
                const Node & this_child = *child[i];
                if ((this_child.tree_solved &&
                    i != max_solved_objective_index) ||
                    (!this_child.tree_solved &&
                        this_child.composite_objective <= max_solved_objective)) {
                    child.erase(child.begin() + i);
                    continue;
                }
            }
        }
        // if path is now solved, mark it as such
        if (solved_children && child.size() == 1) {
            tree_solved = true;
        } else {
            assert(max_unsolved_objective > max_solved_objective);
        }
        // if objective changed, update parents
        double x = std::max(max_unsolved_objective, max_solved_objective);
        if (tree_solved || x > composite_objective) {
            composite_objective = x;
            if (parent != nullptr) {
                parent->UpdateTree();
            }
        }
    }
    // finish battle in which player is still alive
    void FinishBattle() {
        assert(!battle_done);
        assert(hp > 0);
        // no mobs should be present
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            assert(!monster[i].Exists());
        }
        // increase hp by up to 6
        if (relics.burning_blood) {
            Heal(6);
        }
        battle_done = true;
        // calculate objective
        CalculateFinalObjective();
    }
    // heal by the given amount
    void Heal(uint16_t amount) {
        if (amount > max_hp - hp) {
            amount = max_hp - hp;
        }
        if (amount == 0) {
            return;
        }
        hp += amount;
    }
    // reset energy
    void ResetEnergy() {
        energy = 3;
        if (relics.cursed_key) {
            ++energy;
        }
    }
    // reset state to beginning of battle
    void StartBattle() {
        assert(turn == 0);
        //probability = 1.0;
        turn = 1;
        ResetEnergy();
        discard_pile.Clear();
        hand.Clear();
        draw_pile = deck;
        cards_to_draw = 5;
        if (relics.ring_of_the_snake) {
            cards_to_draw += 2;
        }
        if (relics.bag_of_preparation) {
            cards_to_draw += 2;
        }
        if (relics.blood_vial) {
            Heal(2);
        }
        relics.akabeko_active = relics.akabeko;
        if (relics.anchor) {
            block += 10;
        }
        if (relics.ancient_tea_set_active) {
            energy += 2;
            relics.ancient_tea_set_active = 0;
        }
        if (relics.lantern) {
            energy += 1;
        }
        if (relics.oddly_smooth_stone) {
            buff[kBuffDexterity] += 1;
        }
        if (relics.vajra) {
            buff[kBuffStrength] += 1;
        }
        if (relics.bag_of_marbles) {
            for (auto & this_mob : monster) {
                if (!this_mob.Exists()) {
                    continue;
                }
                this_mob.buff.value[kBuffVulnerable] += 1;
            }
        }
        if (relics.bronze_scales) {
            buff[kBuffThorns] += 3;
        }
        player_choice = false;
        generate_mob_intents = true;
        battle_done = false;
    }
    // convert to human readable string
    std::string ToString() const {
        std::ostringstream ss;
        ss << "Game(";
        bool first_item = true;
        if (tree_solved) {
            ss << "solved, ";
            ss.precision(3);
            ss << "objective=" << composite_objective;
            first_item = false;
        }
        if (IsBattleDone()) {
            if (first_item) {
                first_item = false;
            } else {
                ss << ", ";
            }
            if (hp == 0) {
                ss << "dead";
            } else {
                ss << "done";
            }
            if (tree_solved) {
                ss << ")";
                return ss.str();
            }
        }
        // last action
        if (parent != nullptr && parent->player_choice) {
            if (first_item) {
                first_item = false;
            } else {
                ss << ", ";
            }
            ss << parent_decision.ToString();
        }
        if (turn != 0) {
            if (first_item) {
                first_item = false;
            } else {
                ss << ", ";
            }
            ss << "turn=" << turn;
        }
        if (parent != nullptr && !parent->player_choice && parent->child.size() > 1) {
            if (first_item) {
                first_item = false;
            } else {
                ss << ", ";
            }
            ss.precision(3);
            ss << "p=" << probability;
        }
        if (first_item) {
            first_item = false;
        } else {
            ss << ", ";
        }
        ss << "hp=" << hp << "/" << max_hp;
        if (turn == 0) {
            ss << ", deck=" << deck.ToString();
        }
        if (turn != 0 && cards_to_draw) {
            ss << ", to_draw=" << cards_to_draw;
        }
        if (!tree_solved && block) {
            ss << ", block=" << block;
        }
        if (player_choice) {
            ss << ", energy=" << energy;
            ss << ", hand=" << hand.ToString();
        }
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            if (!monster[i].Exists()) {
                continue;
            }
            ss << ", mob" << i << "=(";
            ss << monster[i].base->name << ", " << monster[i].hp << "hp";
            if (monster[i].block) {
                ss << ", block=" << (int) monster[i].block;
            }
            if (!generate_mob_intents) {
                ss << ", " << monster[i].base->intent[monster[i].last_intent[0]].name;
            }
            if (monster[i].buff[kBuffStrength]) {
                ss << ", " << (int) monster[i].buff[kBuffStrength] << "xStr";
            }
            if (monster[i].buff[kBuffVulnerable]) {
                ss << ", " << (int) monster[i].buff[kBuffVulnerable] << "xVuln";
            }
            if (monster[i].buff[kBuffRitual]) {
                ss << ", " << (int) monster[i].buff[kBuffRitual] << "xRitual";
            }
            if (monster[i].buff[kBuffEnrage]) {
                ss << ", " << (int) monster[i].buff[kBuffEnrage] << "xEnrage";
            }
            ss << ")";
        }
        ss << ")";
        return ss.str();
    }
    // print tree
    void PrintTree(
            bool collapse = false,
            std::string indent = "",
            std::string hanging_indent = "") const {
        // if node only has a single child, don't bother printing it
        if (child.size() == 1 && collapse) {
            child[0]->PrintTree(collapse, indent, hanging_indent);
            return;
        }
        // print this node
        std::cout << indent << ToString() << std::endl;
        // print child nodes
        for (std::size_t i = 0; i < child.size(); ++i) {
            auto & this_child = *child[i];
            std::string this_indent = hanging_indent;
            std::string this_hanging_indent = hanging_indent;
            if (i == child.size() - 1) {
                this_indent += "\xc0\xc4"; // "+-";
                this_hanging_indent += "  ";
            } else {
                this_indent += "\xc3\xc4";
                this_hanging_indent += "\xb3 ";
            }
            this_child.PrintTree(collapse, this_indent, this_hanging_indent);
        }
    }
    // mark node as dead
    void Die() {
        battle_done = true;
        CalculateFinalObjective();
    }
    // lose X HP
    void TakeHPLoss(uint16_t hp_loss) {
        // reduce HP
        if (hp <= hp_loss) {
            hp = 0;
            Die();
        } else {
            hp -= hp_loss;
        }
    }
    // take X damage (can be reduced by block)
    void TakeDamage(uint16_t damage) {
        // reduce block
        if (block) {
            if (block >= damage) {
                block -= damage;
                return;
            } else {
                damage -= block;
                block = 0;
            }
        }
        if (damage) {
            TakeHPLoss(damage);
        }
    }
    // get attacked for X damage
    void GetAttacked(uint16_t damage) {
        // apply vulnerability
        if (buff.value[kBuffVulnerable]) {
            damage = (uint16_t) (damage * 1.5);
        }
        TakeDamage(damage);
    }
    // process the end turn action
    // (simulate end of turn and mob actions)
    void EndTurn() {
        // set tree information
        player_choice = false;
        parent_decision.type = kDecisionEndTurn;
        // exhaust all ethereal cards
        for (int i = 0; i < hand.ptr->card.size(); ++i) {
            const Card & card = *card_index[hand.ptr->card[i].first];
            if (card.IsEthereal()) {
                exhaust_pile.AddCard(
                    hand.ptr->card[i].first,
                    hand.ptr->card[i].second);
                hand.RemoveCard(
                    hand.ptr->card[i].first,
                    hand.ptr->card[i].second);
                --i;
            }
        }
        // discard all remaining cards
        discard_pile += hand;
        hand.Clear();
        if (relics.orichalcum && block == 0) {
            block = 6;
        }
        // remove block on mobs
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            auto & mob = monster[i];
            if (!mob.Exists()) {
                continue;
            }
            mob.block = 0;
        }
        // do mob actions
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            auto & mob = monster[i];
            if (!mob.Exists()) {
                continue;
            }
            for (const auto & action : mob.base->intent[mob.last_intent[0]].action) {
                if (action.type == kActionNone) {
                    break;
                }
                switch (action.type) {
                case kActionAttack:
                {
                    int16_t amount = action.arg[0];
                    amount += mob.buff[kBuffStrength];
                    if (mob.buff[kBuffWeak]) {
                        amount = amount * 3 / 4;
                    }
                    if (amount > 0) {
                        GetAttacked(amount);
                        if (IsBattleDone()) {
                            return;
                        }
                    }
                    if (buff.value[kBuffThorns]) {
                        mob.TakeDamage(buff.value[kBuffThorns]);
                        // if this mob died and it's the last one, finish battle
                        if (mob.IsDead() && !MobsAlive()) {
                            FinishBattle();
                            return;
                        }
                    }
                    if (IsBattleDone()) {
                        return;
                    }
                    break;
                }
                case kActionBlock:
                    // mobs are never frail
                    mob.block += action.arg[0];
                    break;
                case kActionBuff:
                    mob.buff[action.arg[0]] += action.arg[1];
                    break;
                case kActionDebuff:
                    buff[action.arg[0]] += action.arg[1];
                    break;
                default:
                    assert(false);
                }
            }
        }
        // cycle mob buffs
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            auto & mob = monster[i];
            if (!mob.Exists()) {
                continue;
            }
            mob.buff.Cycle();
            if (mob.buff[kBuffMetallicize]) {
                mob.Block(mob.buff[kBuffMetallicize]);
            }
        }
        // apply metallicize

        // start new turn
        turn += 1;
        ResetEnergy();
        block = 0;
        generate_mob_intents = true;
        cards_to_draw = 5;
        // cycle player buffs
        buff.Cycle();
    }
    // return true if any mobs are alive
    bool MobsAlive() const {
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            if (monster[i].Exists()) {
                return true;
            }
        }
        return false;
    }
    // play a card
    void PlayCard(uint16_t index, uint8_t target = 0) {
        const auto & card = *card_index[index];
        // set up decision information
        probability = 1.0;
        parent_decision.type = kDecisionPlayCard;
        parent_decision.argument[0] = index;
        parent_decision.argument[1] = target;
        // process enrage
        if (card.type == kCardTypeSkill) {
            for (auto & mob : monster) {
                if (mob.buff[kBuffEnrage]) {
                    mob.buff[kBuffStrength] += mob.buff[kBuffEnrage];
                }
            }
        }
        // pay for it
        assert(energy >= card.cost);
        energy -= card.cost;
        auto & mob = monster[target];
        if (card.IsTargeted()) {
            assert(mob.Exists());
        }
        // do actions
        for (auto & action : card.action) {
            if (action.type == kActionNone) {
                break;
            }
            switch (action.type) {
                case kActionAttack:
                {
                    assert(card.IsTargeted());
                    uint16_t amount = action.arg[0];
                    amount += buff[kBuffStrength];
                    if (relics.akabeko_active) {
                        relics.akabeko_active = 0;
                        amount += 8;
                    }
                    if (buff[kBuffWeak]) {
                        amount = amount * 3 / 4;
                    }
                    for (int16_t i = 0; i < action.arg[1]; ++i) {
                        if (mob.Exists()) {
                            mob.Attack(amount);
                            if (mob.buff[kBuffThorns]) {
                                GetAttacked(mob.buff[kBuffThorns]);
                                // if we died, we're done
                                if (hp == 0) {
                                    return;
                                }
                            }
                            // if this mob died and it's the last one, finish battle
                            if (mob.IsDead() && !MobsAlive()) {
                                FinishBattle();
                                return;
                            }
                        }
                    }
                    break;
                }
                case kActionAttackAll:
                {
                    assert(!card.IsTargeted());
                    uint16_t amount = action.arg[0];
                    amount += buff[kBuffStrength];
                    if (relics.akabeko_active) {
                        relics.akabeko_active = 0;
                        amount += 8;
                    }
                    for (int16_t i = 0; i < action.arg[1]; ++i) {
                        for (int16_t m = 0; m < MAX_MOBS_PER_NODE; ++m) {
                            auto & this_mob = monster[m];
                            if (this_mob.Exists()) {
                                this_mob.Attack(amount);
                                if (this_mob.buff[kBuffThorns]) {
                                    GetAttacked(this_mob.buff[kBuffThorns]);
                                    // if we died, we're done
                                    if (hp == 0) {
                                        return;
                                    }
                                }
                                // if this mob died and it's the last one, finish battle
                                if (this_mob.IsDead() && !MobsAlive()) {
                                    FinishBattle();
                                    return;
                                }
                            }
                        }
                    }
                    break;
                }
                case kActionBlock:
                {
                    int16_t x = action.arg[0] + buff[kBuffDexterity];
                    if (x > 0) {
                        block += x;
                    }
                    break;
                }
                case kActionBuff:
                    buff[action.arg[0]] += action.arg[1];
                    break;
                case kActionDebuff:
                    assert(card.IsTargeted());
                    if (mob.Exists()) {
                        mob.buff[action.arg[0]] += action.arg[1];
                    }
                    break;
                case kActionDebuffAll:
                    assert(!card.IsTargeted());
                    for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
                        if (!monster[i].Exists()) {
                            continue;
                        }
                        monster[i].buff[action.arg[0]] += action.arg[1];
                    }
                    break;
                case kActionAddCardToDrawPile:
                    draw_pile.AddCard(action.arg[0]);
                    break;
                case kActionAddCardToDiscardPile:
                    discard_pile.AddCard(action.arg[0]);
                    break;
                default:
                    assert(false);
            }
        }
    }
    // copy the given node and return a reference to the copy
    Node & CreateChild(std::list<Node> & all_nodes) {
        all_nodes.push_back(*this);
        Node & new_node = *all_nodes.rbegin();
        new_node.child.clear();
        new_node.parent = this;
        AddChild(new_node);
        return new_node;
    }
    // return true if this node is strictly worse or equal to the given node
    bool IsWorseOrEqual(const Node & that) const {
        if (that.IsBattleDone() &&
                that.GetMaxFinalObjective() >= GetMaxFinalObjective()) {
            return true;
        }
        if (IsBattleDone()) {
            return false;
        }
        // after this point, neither battle is complete
        if (hp > that.hp) {
            return false;
        }
        if (block > that.block) {
            return false;
        }
        if (energy > that.energy) {
            return false;
        }
        if (cards_to_draw != that.cards_to_draw) {
            return false;
        }
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            auto & mob = monster[i];
            auto & that_mob = that.monster[i];
            if (!mob.Exists()) {
                if (that_mob.Exists()) {
                    return false;
                } else {
                    continue;
                }
            }
            if (mob.hp < that_mob.hp) {
                return false;
            }
            if (!mob.buff.MobIsWorseOrEqual(that_mob.buff)) {
                return false;
            }
        }
        if (!buff.PlayerIsWorseOrEqual(that.buff)) {
            return false;
        }
        return true;
    }
    //// find all nodes in which we can play cards
    //void FindChoiceNodes(
    //        std::list<Node> & all_nodes,
    //        std::deque<Node *> & recent_node) {
    //    //std::cout << "\nExpanding: " << ToString() << "\n";
    //    std::vector<Node *> hanging_nodes;
    //    player_choice = true;
    //    Node * best_path = nullptr;
    //    hanging_nodes.push_back(this);
    //    // nodes at which the player no longer has a choice
    //    // (e.g. after pressing end turn or after player or all mobs are dead)
    //    std::vector<Node *> terminal_node;
    //    while (!hanging_nodes.empty()) {
    //        // loop through each node we need to expand
    //        std::vector<Node *> new_hanging_nodes;
    //        for (auto & this_node_ptr : hanging_nodes) {
    //            Node & this_node = *this_node_ptr;
    //            // add end the turn node
    //            Node & new_node = this_node.CreateChild(all_nodes);
    //            new_node.parent_decision.type = kDecisionEndTurn;
    //            new_node.EndTurn();
    //            if (new_node.hp == 0) {
    //                printf("Dead!\n");
    //            }
    //            if (new_node.IsBattleDone() &&
    //                new_node.GetMaxFinalObjective() == GetMaxFinalObjective()) {
    //                best_path = &new_node;
    //                break;
    //            }
    //            terminal_node.push_back(&new_node);
    //            // play all possible cards
    //            for (std::size_t i = 0; i < this_node.hand.card.size(); ++i) {
    //                // skip this card if it's too expensive
    //                auto & card = *card_index[this_node.hand.card[i].first];
    //                if (card.cost > this_node.energy) {
    //                    continue;
    //                }
    //                // play this card
    //                if (card.targeted) {
    //                    for (int m = 0; m < 5; ++m) {
    //                        if (!this_node.monster[m].Exists()) {
    //                            continue;
    //                        }
    //                        Node & new_node = this_node.CreateChild(all_nodes);
    //                        new_node.parent_decision.type = kDecisionPlayCard;
    //                        uint16_t index = this_node.hand.card[i].first;
    //                        new_node.parent_decision.argument[0] = index;
    //                        new_node.hand.RemoveCard(index);
    //                        new_node.PlayCard(index, m);
    //                        // if this is the best possible objective,
    //                        // don't process any further choices
    //                        if (new_node.IsBattleDone() &&
    //                                new_node.GetMaxFinalObjective() == GetMaxFinalObjective()) {
    //                            best_path = &new_node;
    //                            break;
    //                        }
    //                        // add to exhaust or discard pile
    //                        if (card.exhausts) {
    //                            new_node.exhaust_pile.AddCard(index);
    //                        } else {
    //                            new_node.discard_pile.AddCard(index);
    //                        }
    //                        // add new decision point
    //                        new_hanging_nodes.push_back(&new_node);
    //                    }
    //                } else {
    //                    Node & new_node = this_node.CreateChild(all_nodes);
    //                    new_node.parent_decision.type = kDecisionPlayCard;
    //                    uint16_t index = this_node.hand.card[i].first;
    //                    new_node.parent_decision.argument[0] = index;
    //                    new_node.hand.RemoveCard(index);
    //                    new_node.PlayCard(index);
    //                    // if this is the best possible objective,
    //                    // don't process any further choices
    //                    if (new_node.IsBattleDone() &&
    //                        new_node.GetMaxFinalObjective() == GetMaxFinalObjective()) {
    //                        best_path = &new_node;
    //                        break;
    //                    }
    //                    if (card.exhausts) {
    //                        new_node.exhaust_pile.AddCard(index);
    //                    } else {
    //                        new_node.discard_pile.AddCard(index);
    //                    }
    //                    new_hanging_nodes.push_back(&new_node);
    //                }
    //                if (best_path != nullptr) {
    //                    break;
    //                }
    //            }
    //            if (best_path != nullptr) {
    //                break;
    //            }
    //        }
    //        if (best_path != nullptr) {
    //            break;
    //        }
    //        hanging_nodes = new_hanging_nodes;
    //    }
    //    // if we have a definitive best path, eliminate all other choices
    //    if (best_path != nullptr) {
    //        //std::cout << "Found definitive best path\n";
    //        //std::cout << "--> " << best_path->ToString() << "\n";
    //        Node * node = best_path;
    //        while (node != this) {
    //            // delete all other children except for this one
    //            if (node->parent->child.size() != 1) {
    //                node->parent->child.clear();
    //                node->parent->child.push_back(node);
    //            }
    //            node = node->parent;
    //        }
    //        return;
    //    }
    //    // find nodes which are equal or worse than another node and remove them
    //    std::vector<bool> bad_node(terminal_node.size(), false);
    //    for (std::size_t i = 0; i < terminal_node.size(); ++i) {
    //        Node & node_i = *terminal_node[i];
    //        if (bad_node[i]) {
    //            continue;
    //        }
    //        for (std::size_t j = 0; j < terminal_node.size(); ++j) {
    //            if (i == j || bad_node[j]) {
    //                continue;
    //            }
    //            Node & node_j = *terminal_node[j];
    //            if (node_j.IsWorseOrEqual(node_i)) {
    //                bad_node[j] = true;
    //            }
    //        }
    //    }
    //    if (false && terminal_node.size() > 1) {
    //        for (std::size_t i = 0; i < terminal_node.size(); ++i) {
    //            if (!bad_node[i]) {
    //                std::cout << "--> ";
    //            }
    //            std::cout << terminal_node[i]->ToString() << "\n";
    //        }
    //    }
    //    // at least one node must be good
    //    assert(std::find(bad_node.begin(), bad_node.end(), false) != bad_node.end());
    //    // remove bad choices and their parents where possible
    //    for (std::size_t i = 0; i < terminal_node.size(); ++i) {
    //        if (!bad_node[i]) {
    //            continue;
    //        }
    //        // remove this node from its parent and prune empty nodes
    //        Node * node_ptr = terminal_node[i];
    //        while (node_ptr != this && node_ptr->child.empty()) {
    //            auto & vec = node_ptr->parent->child;
    //            auto it = std::find(vec.begin(), vec.end(), node_ptr);
    //            assert(it != vec.end());
    //            vec.erase(it);
    //            node_ptr = node_ptr->parent;
    //        }
    //    }
    //    // add good terminal nodes unless they're done
    //    for (std::size_t i = 0; i < terminal_node.size(); ++i) {
    //        if (bad_node[i]) {
    //            continue;
    //        }
    //        if (!terminal_node[i]->IsBattleDone()) {
    //            recent_node.push_back(terminal_node[i]);
    //        }
    //    }
    //}
    // find solved terminal leaves and put them in a list along with
    // probability to reach each one
    void FindTerminalLeaves(
            double p,
            std::list<std::pair<double, Node *>> &end_states) {
        if (child.empty() && tree_solved) {
            end_states.push_back(std::pair<double, Node *>(p, this));
        } else {
            for (auto & this_ptr : child) {
                double new_p = p;
                if (!player_choice) {
                    new_p *= this_ptr->probability;
                }
                this_ptr->FindTerminalLeaves(new_p, end_states);
            }
        }
    }
    // verify this node and return true if everything checks out
    bool Verify() {
        bool pass = true;
        if (child.empty()) {
            return true;
        }
        if (player_choice && tree_solved) {
            assert(child.size() == 1);
            assert(child[0]->probability == 1.0);
        }
        double p = 0.0;
        for (auto & ptr : child) {
            if (player_choice) {
                assert(ptr->probability == 1.0);
            }
            pass = pass && ptr->Verify();
            p += ptr->probability;
        }
        if (!player_choice) {
            if (abs(p - 1.0) > 1e-10) {
                pass = false;
                printf("ERROR: probability (%g) != 1\n", p);
                PrintTree();
            }
        }
        // if all children are solved, this should be solved as well
        if (!child.empty()) {
            bool children_solved = true;
            for (auto & it : child) {
                if (!it->tree_solved) {
                    children_solved = false;
                }
            }
            if (tree_solved && !children_solved) {
                PrintTree();
                printf("ERROR: incorrectly marked solved\n");
                pass = false;
            }
            if (!tree_solved && children_solved) {
                PrintTree();
                printf("ERROR: incorrectly not marked solved\n");
                pass = false;
            }
        }
        return pass;
    }
    // print out completed tree stats
    void PrintStats() {
        std::cout << "Top node: " << ToString() << "\n";
        std::cout << "Children:\n";
        for (auto it : child) {
            std::cout << "- " << it->ToString() << "\n";
        }
        std::cout << "\n";
        std::cout << "Solution tree has " << CountNodes() << " nodes.\n";
        // get a list of final states and probability of reaching each one
        std::list<std::pair<double, Node *>> end_states;
        // walk tree
        FindTerminalLeaves(1.0, end_states);
        double check = 0.0;
        for (auto & ptr : end_states) {
            check += ptr.first;
        }
        if (abs(check - 1.0) > 1e-10) {
            printf("ERROR: total probability (%g) not 1\n", check);
        }
        // probability of dying
        double death_chance = 0.0;
        // find probability of each ending objectives
        std::map<double, double> final_objective;
        // find distribution of turns
        std::map<uint16_t, double> battle_length;
        // walk through tree to populate above stats
        for (auto & ptr : end_states) {
            battle_length[ptr.second->turn] += ptr.first;
            final_objective[ptr.second->composite_objective] += ptr.first;
            if (ptr.second->composite_objective == 0) {
                death_chance += ptr.first;
            }
        }
        // get average length
        double average_turns = 0;
        for (auto & it : battle_length) {
            average_turns += it.first * it.second;
        }
        std::cout << "Average battle length is " << average_turns
            << " (" << battle_length.rbegin()->first << " in longest case)\n";
        double average_objective = 0;
        for (auto & it : final_objective) {
            average_objective += it.first * it.second;
        }
        std::cout << "Average final objective is " << average_objective
            << " (" << final_objective.begin()->first << " in worst case)\n";
        std::cout << "Death in " << (death_chance * 100) << "% of cases\n";
    }
};

// output to a stringstream
std::stringstream & operator<< (std::stringstream & out, const Node & node) {
    out << node.ToString();
    return out;
}
