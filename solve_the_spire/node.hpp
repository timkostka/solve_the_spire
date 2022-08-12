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
#include "cards.hpp"
#include "buff_state.hpp"
#include "monster.hpp"
#include "relics.hpp"
#include "fight.hpp"
#include "orbs.hpp"

// enum for a decision
enum DecisionTypeEnum : uint8_t {
    kDecisionUnused = 0,
    // play a card
    // first argument is card index played
    // second argument is target (if applicable)
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
            const Card & card = *card_map[argument[0]];
            ss << "play " << card.name;
        } else if (type == kDecisionEndTurn) {
            return "end turn";
        } else {
            ss << "unknown";
        }
        return ss.str();
    }
};

// bitfield for holding some Node flags
struct NodeFlagStruct {
    // true if battle is complete
    // (battle is complete if all mobs are dead or if player is dead)
    bool battle_done : 1;
    // true if tree below this is solved
    bool tree_solved : 1;
    // true if last card played was an attack
    bool last_card_attack : 1;
    // true if last card played was a skill
    bool last_card_skill : 1;
};

// A Node contains all information about a game node.
struct Node {
    // set to true at tree start if we have cards where the last skill played matters
    static bool last_card_skill_matters;
    // set to true at tree start if we have cards where the last attack played matters
    static bool last_card_attack_matters;
    // turn number
    uint8_t turn;
    // player energy
    uint8_t energy;
    // layer number (for evaluating tree)
    // increase by 1 for each decision that happens so that we can evaluate
    // the lowest layer to keep memory requirements low
    uint8_t layer;
    // max player HP
    uint8_t max_hp;
    // current player HP
    uint8_t hp;
    // amount of block
    uint8_t block;
    // stance
    StanceEnum stance;
    // variable flags
    NodeFlagStruct flag;
#ifdef USE_ORBS
    // focus
    uint8_t focus;
    // orb slots
    uint8_t orb_slots;
    // orbs
    std::vector<OrbStruct> orbs;
#endif
    // pointer to deck
    static CardCollectionPtr deck;
    // hand
    CardCollectionPtr hand;
    // draw pile
    CardCollectionPtr draw_pile;
    // discard pile
    CardCollectionPtr discard_pile;
    // exhausted pile
    CardCollectionPtr exhaust_pile;
    // pointer to parent node
    Node * parent;
    // monsters (in order of action)
    Monster monster[MAX_MOBS_PER_NODE];
    // pre-actions
    Action pending_action[MAX_PENDING_ACTIONS];
    // buffs
    BuffState buff;
    // relic state
    RelicStruct relics;
    // probability of getting to this node if we make the right choices
    double probability;
    // decision at parent node in order to get to this node
    // (only valid if parent has no pending actions)
    Decision parent_decision;
    // maximum possible composite objective
    // (if tree_solved=true, this is the final composite objective)
    double objective;
    // list of children
    vector<Node *> child;
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
        return flag.battle_done;
    }
#ifdef USE_ORBS
    // pop the rightmost orb
    void PopOrb() {
        orbs.erase(orbs.begin());
    }
    // evoke an orb
    void EvokeOrb(OrbStruct & orb) {
        if (orb.type == kOrbLightning) {
            if (MAX_MOBS_PER_NODE > 1 && monster[1].Exists()) {
                printf("Random targeting with 2+ enemies not implemented\n");
                exit(1);
            }
            if (monster[0].Exists()) {
                monster[0].TakeDamage(8 + focus, false);
            }
        } else if (orb.type == kOrbFrost) {
            block += 5 + focus;
        } else if (orb.type == kOrbDark) {
            if (MAX_MOBS_PER_NODE > 1 && monster[1].Exists()) {
                printf("Random targeting with 2+ enemies not implemented\n");
                exit(1);
            }
            if (monster[0].Exists()) {
                monster[0].TakeDamage(orb.damage, false);
            }
        } else if (orb.type == kOrbFusion) {
            energy += 2;
        }
    }
    // channel an orb
    void ChannelOrb(OrbEnum type) {
        if (orbs.size() == orb_slots) {
            EvokeOrb(orbs[0]);
            PopOrb();
        }
        orbs.push_back(OrbStruct(type));
    }
    // process fusion orbs passives at start of turn
    void ProcessOrbsStartTurn() {
        for (auto & orb : orbs) {
            if (orb.type == kOrbFusion) {
                energy += 1;
            }
        }
    }
    // process all orb passives at end of turn
    void ProcessOrbsEndTurn() {
        for (auto & orb : orbs) {
            if (orb.type == kOrbLightning) {
                // only implemented for 1 mob alive
                if (MAX_MOBS_PER_NODE > 1 && monster[1].Exists()) {
                    printf("Random targeting with 2+ enemies not implemented\n");
                    exit(1);
                }
                if (monster[0].Exists()) {
                    monster[0].TakeDamage(3 + focus, false);
                }
            } else if (orb.type == kOrbFrost) {
                block += 2 + focus;
            } else if (orb.type == kOrbDark) {
                orb.damage += 6 + focus;
            }
        }
    }
#endif
    // return best possible ultimate objective function for a child of this node
    // (it's okay to overestimate, but not ideal)
    double GetMaxFinalObjective() const {
        assert(!flag.battle_done);
        // TODO: factor in Bandage Up
        if (IsBattleDone()) {
            return hp;
        }
        uint16_t top = hp;
        if (relics.meat_on_the_bone) {
            uint16_t meat_max = hp / 2 + 12;
            if (top < meat_max) {
                top = meat_max;
            }
        }
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
        flag.tree_solved = true;
        objective = hp;
        if (hp == 0) {
            for (auto & mob : monster) {
                if (mob.Exists()) {
                    objective -= mob.hp / 1000.0;
                }
            }
        }
    }
    // return an objective function used to evaluate the best decision
    // this function may only use information within this node--no information
    // from children is allowed
    double GetPathObjective() {
        // DEBUG
        // this is illegal, isn't it? Since it looks at children?
        //return CalculateObjective() + 1000.0 * layer;
        double x = 5.0 * hp;
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            if (monster[i].Exists()) {
                x += monster[i].max_hp - monster[i].hp;
            }
        }
        // favor evaluating later turns
        x += 1000.0 * layer;
        //x += turn * 1000;
        return x;
    }
    // initialize a start of fight node
    // (only things that need initialized outside of this are:
    // hp, max_hp, fight_type, deck
    void InitializeStartingNode() {
        assert(hp > 0);
        assert(max_hp >= hp);
        // TODO: populate last_card_attack/skill_matters based on cards in deck
        stance = kStanceNone;
        discard_pile.Clear();
        hand.Clear();
        exhaust_pile.Clear();
        draw_pile = deck;
        flag.last_card_attack = false;
        flag.last_card_skill = false;
#ifdef USE_ORBS
        focus = 0;
#endif
        for (auto & action : pending_action) {
            action.type = kActionNone;
        }
        pending_action[0].type = kActionGenerateBattle;
        layer = 0;
        turn = 0;
        block = 0;
        probability = 1.0;
        energy = 0;
        parent = nullptr;
        buff.Reset();
        flag.tree_solved = false;
        flag.battle_done = false;
        objective = GetMaxFinalObjective();
        //path_objective = GetPathObjective();
    }
    // pop the first pending action
    void PopPendingAction() {
        for (std::size_t i = 1; i < MAX_PENDING_ACTIONS; ++i) {
            pending_action[i - 1] = pending_action[i];
        }
        pending_action[MAX_PENDING_ACTIONS - 1].type = kActionNone;
        pending_action[MAX_PENDING_ACTIONS - 1].arg[0] = 0;
        pending_action[MAX_PENDING_ACTIONS - 1].arg[1] = 0;
    }
    // return true if node has pending actions
    bool HasPendingActions() const {
        return pending_action[0].type != kActionNone;
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
        if (flag.tree_solved) {
            return 0;
        }
        if (child.empty()) {
            return (flag.tree_solved) ? 0 : 1;
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
    // return true if this has children
    bool HasChildren() const {
        return !child.empty();
    }
    // estimate the final objective based on the nodes that are solved so far
    // result is (p, obj) where p is percentage of solved nodes
    std::pair<double, double> EstimateFinalObjective() const {
        // if tree is solved, we know exactly what objective was
        if (flag.tree_solved) {
            return std::pair<double, double>(probability, probability * objective);
        }
        std::pair<double, double> result(0.0, 0.0);
        if (HasChildren()) {
            double multiplier = 1.0;
            if (!HasPendingActions()) {
                multiplier /= child.size();
            }
            for (auto & this_child_ptr : child) {
                auto this_result = this_child_ptr->EstimateFinalObjective();
                if (this_result.first > 0.0) {
                    result.first += multiplier * this_result.first;
                    result.second += multiplier * this_result.second;
                }
            }
        }
        return result;
    }
    // return completion percent of this node
    // 1.0 = all children done
    double GetSolvedCompletionPercent() const {
        if (flag.tree_solved) {
            return 1.0;
        }
        if (HasChildren()) {
            double p = 0.0;
            double dp = 1.0 / child.size();
            for (auto & this_child_ptr : child) {
                p += dp * this_child_ptr->GetSolvedCompletionPercent();
            }
            return p;
        } else {
            return 0.0;
        }
    }
    // return the number of links between this and the given ancestor
    uint16_t CountLevelsBelow(const Node & that) const {
        const Node * node_ptr = this;
        uint16_t level = 0;
        while (node_ptr != &that) {
            if (node_ptr->parent == nullptr) {
                return -1;
            }
            if (node_ptr->IsOrphan()) {
                return -1;
            }
            node_ptr = node_ptr->parent;
            ++level;
        }
        return level;
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
    // return an estimate for the composite objective by looking at direct children
    // and also update solved state
    double CalculateObjective() const {
        // if no children, just get max
        if (child.empty()) {
            return objective;
            //return GetMaxFinalObjective();
        }
        // if one child, just return objective from that child
        if (child.size() == 1) {
            return child[0]->objective;
        }
        // if children and players choice, return max
        if (!HasPendingActions()) {
            double max_objective = child[0]->objective;
            for (std::size_t i = 1; i < child.size(); ++i) {
                if (child[i]->objective > max_objective) {
                    max_objective = child[i]->objective;
                }
            }
            return max_objective;
        } else {
            // if not player choice, then return probability adjusted composite
            double total_probability = child[0]->probability;
            double objective = child[0]->probability * child[0]->objective;
            double max_objective = child[0]->objective;
            double min_objective = max_objective;
            for (std::size_t i = 1; i < child.size(); ++i) {
                total_probability += child[i]->probability;
                objective += child[i]->probability * child[i]->objective;
                if (child[i]->objective > max_objective) {
                    max_objective = child[i]->objective;
                } else if (child[i]->objective < min_objective) {
                    min_objective = child[i]->objective;
                }
            }
            // to prevent roundoff errors from propagating
            if (max_objective == min_objective) {
                objective = max_objective;
            } else {
                objective /= total_probability;
            }
            return objective;
        }
    }
    // evaluate the composite objective of this node and all its descendents
    void CalculateObjectiveOfChildren() {
        for (auto & child_ptr : child) {
            child_ptr->CalculateObjectiveOfChildren();
            if (!child_ptr->flag.tree_solved) {
                child_ptr->objective = child_ptr->CalculateObjective();
            }
        }
        // if not solved, calculate this composite objective
        if (!flag.tree_solved) {
            objective = CalculateObjective();
        }
        // if all children solved, mark this solved
        if (child.size() == 1 && child[0]->flag.tree_solved) {
            flag.tree_solved = true;
        }
    }
    // return true if all children are solved
    bool AreChildrenSolved() {
        assert(!child.empty());
        for (auto & it : child) {
            if (!it->flag.tree_solved) {
                return false;
            }
        }
        return true;
    }
    // finish battle in which player is still alive
    void FinishBattle() {
        assert(!flag.battle_done);
        assert(hp > 0);
        // no mobs should be present
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            assert(!monster[i].Exists());
        }
        if (relics.meat_on_the_bone && hp * 2 <= max_hp) {
            Heal(12);
        }
        if (relics.burning_blood) {
            Heal(6);
        }
        flag.battle_done = true;
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
    // add energy at start of battle
    void ResetEnergy() {
        if (!relics.ice_cream) {
            energy = 0;
        }
        energy += 3;
        if (relics.cursed_key) {
            ++energy;
        }
        if (relics.coffee_dripper) {
            ++energy;
        }
        energy += buff[kBuffBerserk];
    }
    // reset state to beginning of battle
    void StartBattle() {
        assert(turn == 0);
        assert(energy == 0);
        turn = 1;
        ResetEnergy();
        int16_t cards_to_draw = 5;
        if (relics.pure_water) {
            hand.AddCard(card_miracle);
        }
        if (relics.ring_of_the_snake) {
            cards_to_draw += 2;
        }
#ifdef USE_ORBS
        if (relics.cracked_core) {
            ChannelOrb(kOrbLightning);
        }
#endif
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
        // add draw card and generate mob intent preactions
        assert(!HasPendingActions());
        pending_action[0].type = kActionGenerateMobIntents;
        pending_action[0].arg[0] = 0;
        pending_action[0].arg[1] = 0;
        pending_action[1].type = kActionDrawCards;
        pending_action[1].arg[0] = cards_to_draw;
        pending_action[1].arg[1] = 0;
        flag.battle_done = false;
    }
    // convert to human readable string
    std::string ToString() const {
        std::ostringstream ss;
        ss << "Game(";
        bool first_item = true;
        ss.precision(6);
        if (flag.tree_solved) {
            ss << "solved, ";
            ss << "obj=" << objective;
            first_item = false;
        } else {
            ss << "maxobj=" << objective;
            first_item = false;
        }
        ss.precision(3);
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
        }
        // last action
        if (parent != nullptr && !parent->HasPendingActions()) {
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
            ss << "turn=" << (int) turn;
        }
        if (first_item) {
            first_item = false;
        } else {
            ss << ", ";
        }
        ss.precision(3);
        ss << "p=" << probability;
        if (first_item) {
            first_item = false;
        } else {
            ss << ", ";
        }
        ss << "hp=" << (int) hp << "/" << (int) max_hp;
        if (stance == kStanceWrath) {
            ss << ", Wrath";
        } else if (stance == kStanceCalm) {
            ss << ", Calm";
        }
        if (turn == 0) {
            ss << ", deck=" << deck.ToString();
            //ss << ", fight=" << fight_map[fight_type].name;
        }
        if (turn == 0) {
        }
        if (turn != 0 && pending_action[0].type == kActionDrawCards) {
            ss << ", to_draw=" << (int) pending_action[0].arg[0];
        }
        if (block) {
            ss << ", block=" << (int) block;
        }
        if (!HasPendingActions()) {
            ss << ", energy=" << (int) energy;
            ss << ", hand=" << hand.ToString();
        } else if (!hand.IsEmpty()) {
            ss << ", hand=" << hand.ToString();
        }
#ifdef USE_ORBS
        if (!orbs.empty()) {
            ss << ", orbs=";
            ss << orbs[0].ToString();
            for (int i = 1; i < orbs.size(); ++i) {
                ss << "," << orbs[i].ToString();
            }
        }
#endif
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            if (!monster[i].Exists()) {
                continue;
            }
            ss << ", mob" << i << "=(";
            ss << monster[i].base->name << ", " << monster[i].hp << "hp";
            if (monster[i].block) {
                ss << ", block=" << (int) monster[i].block;
            }
            if (pending_action[0].type != kActionGenerateMobIntents &&
                    pending_action[1].type != kActionGenerateMobIntents) {
                ss << ", " << monster[i].base->intent[monster[i].last_intent[0]].name;
            }
            if (monster[i].buff[kBuffStrength]) {
                ss << ", " << (int) monster[i].buff[kBuffStrength] << "xStr";
            }
            if (monster[i].buff[kBuffMetallicize]) {
                ss << ", " << (int) monster[i].buff[kBuffMetallicize] << "xMetallicize";
            }
            if (monster[i].buff[kBuffRegenerate]) {
                ss << ", " << (int) monster[i].buff[kBuffRegenerate] << "xRegen";
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
            Node * highlight = nullptr,
            std::string indent = "",
            std::string hanging_indent = "") const {
        // if node only has a single child, don't bother printing it
        if (child.size() == 1 && collapse) {
            child[0]->PrintTree(collapse, highlight, indent, hanging_indent);
            return;
        }
        // print this node
        if (highlight != nullptr) {
            if (this == highlight) {
                std::cout << "--> ";
            } else {
                std::cout << "    ";
            }
        }
        std::cout << indent << ToString() << std::endl;
        // print child nodes
        for (std::size_t i = 0; i < child.size(); ++i) {
            auto & this_child = *child[i];
            std::string this_indent = hanging_indent;
            std::string this_hanging_indent = hanging_indent;
            if (i == child.size() - 1) {
                //this_indent += "\xc0\xc4"; // "+-";
                this_indent += "+-";
                this_hanging_indent += "  ";
            } else {
                //this_indent += "\xc3\xc4";
                this_indent += "+-";
                //this_hanging_indent += "\xb3 ";
                this_hanging_indent += "| ";
            }
            this_child.PrintTree(collapse, highlight, this_indent, this_hanging_indent);
        }
    }
    // mark node as dead
    void Die() {
        flag.battle_done = true;
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
        parent_decision.type = kDecisionEndTurn;
        // process orbs
#ifdef USE_ORBS
        ProcessOrbsEndTurn();
#endif
        if (!MobsAlive()) {
            FinishBattle();
            return;
        }
        // exhaust all ethereal cards
        {
            auto & original_hand = hand.node_ptr->collection.card;
            for (auto & deck_item : original_hand) {
                const Card & card = *card_map[deck_item.first];
                if (card.flag.ethereal) {
                    exhaust_pile.AddCard(deck_item);
                    hand.RemoveCard(deck_item);
                }
            }
        }
        // take burn damage and discard burns
        {
            static card_index_t burn_card_index = card_burn.GetIndex();
            card_count_t burn_count = hand.CountCard(burn_card_index);
            for (card_count_t i = 0; i < burn_count; ++i) {
                TakeDamage(2);
            }
            if (burn_count) {
                discard_pile.AddCard(burn_card_index, burn_count);
                hand.RemoveCard(burn_card_index, burn_count);
            }
        }
        // discard all remaining cards except those we retain
        if (!relics.runic_pyramid) {
            CardCollectionPtr new_hand;
            for (auto & deck_item : hand) {
                const Card & card = *card_map[deck_item.first];
                if (card.flag.retain) {
                    new_hand.AddCard(deck_item.first, deck_item.second);
                } else {
                    discard_pile.AddCard(deck_item.first, deck_item.second);
                }
            }
            // remove them from hand
            hand = new_hand;
        }
        // combust
        if (buff[kBuffCombustHpLoss]) {
            TakeDamage(buff[kBuffCombustHpLoss]);
            for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
                auto & mob = monster[i];
                if (!mob.Exists()) {
                    continue;
                }
                mob.TakeDamage(buff[kBuffCombustDamage], false);
            }
            if (!MobsAlive()) {
                FinishBattle();
                return;
            }
        }
        // orichalcum
        if (relics.orichalcum && block == 0) {
            block = 6;
        }
        // metallicize
        if (buff[kBuffMetallicize]) {
            block += buff[kBuffMetallicize];
        }
        // reduce no draw
        if (buff[kBuffNoDraw]) {
            --buff[kBuffNoDraw];
        }

        ///////////////////////
        // START OF MOB TURN //
        ///////////////////////

        // apply poison to mobs
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            auto & mob = monster[i];
            if (!mob.Exists()) {
                continue;
            }
            // apply noxious fumes poison
            if (buff[kBuffNoxiousFumes]) {
                mob.buff[kBuffPoison] += buff[kBuffNoxiousFumes];
            }
        }

        // remove block on mobs
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            auto & mob = monster[i];
            if (!mob.Exists()) {
                continue;
            }
            // remove block
            if (!mob.buff[kBuffBarricade]) {
                mob.block = 0;
            }
            // take poison
            if (mob.buff[kBuffPoison]) {
                mob.TakeHPLoss(mob.buff[kBuffPoison], false);
                --mob.buff[kBuffPoison];
                if (!mob.Exists()) {
                    continue;
                }
            }
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
                    if (stance == kStanceWrath) {
                        amount *= 2;
                    }
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
                        mob.TakeDamage(buff.value[kBuffThorns], false);
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
                    printf("ERROR: unexpected value\n");
                    exit(1);
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
            if (mob.buff[kBuffRegenerate]) {
                mob.hp += mob.buff[kBuffRegenerate];
                if (mob.hp > mob.max_hp) {
                    mob.hp = mob.max_hp;
                }
            }
        }
        // start new turn
        turn += 1;
        // cycle energy
        ResetEnergy();
        // reset block
        if (buff[kBuffBarricade]) {
        } else if (block > 15 && relics.calipers) {
            block -= 15;
        } else {
            block = 0;
        }
        assert(!HasPendingActions());
        // player should never have poison
        assert(!buff[kBuffPoison]);
        pending_action[0].type = kActionGenerateMobIntents;
        pending_action[0].arg[0] = 0;
        pending_action[0].arg[1] = 0;
        pending_action[1].type = kActionDrawCards;
        pending_action[1].arg[0] = 5;
        pending_action[1].arg[1] = 0;
        // brutality buff
        if (buff[kBuffBrutality]) {
            TakeHPLoss(buff[kBuffBrutality]);
            if (IsBattleDone()) {
                return;
            }
            pending_action[0].arg[0] += buff[kBuffBrutality];
        }
        // TODO: factor in relics that increase cards to draw
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
    // return the number of "Strike" cards present
    uint16_t CountStrikeCards() {
        uint16_t count = 0;
        for (auto & deck_item : draw_pile) {
            if (card_map[deck_item.first]->flag.strike) {
                count += deck_item.second;
            }
        }
        for (auto & deck_item : draw_pile) {
            if (card_map[deck_item.first]->flag.strike) {
                count += deck_item.second;
            }
        }
        for (auto & deck_item : draw_pile) {
            if (card_map[deck_item.first]->flag.strike) {
                count += deck_item.second;
            }
        }
        return count;
    }
    // sort mobs
    void SortMobs() {
        for (int i = 0; i < MAX_MOBS_PER_NODE - 1; ++i) {
            if (!monster[i].Exists() && monster[i + 1].Exists()) {
                monster[i] = monster[i + 1];
                monster[i + 1].hp = 0;
                monster[i + 1].base = nullptr;
            }
        }
    }
    // play a card
    // target is mob index or card in hand index
    void PlayCard(card_index_t index, uint8_t target = 0) {
        const auto & card = *card_map[index];
        // set up decision information
        parent_decision.type = kDecisionPlayCard;
        parent_decision.argument[0] = index;
        parent_decision.argument[1] = target;
        // process enrage
        if (card.flag.skill) {
            for (auto & mob : monster) {
                if (mob.buff[kBuffEnrage]) {
                    mob.buff[kBuffStrength] += mob.buff[kBuffEnrage];
                }
            }
        }
        // pay for it
        assert(energy >= card.cost);
        uint16_t card_energy = card.cost;
        if (card.flag.x_cost) {
            card_energy = energy;
        }
        energy -= card_energy;
        auto & mob = monster[target];
        if (card.flag.targeted) {
            assert(mob.Exists());
        }
        // do actions
        for (unsigned int i = 0; i < MAX_CARD_ACTIONS; ++i) {
            const auto & action = card.action[i];
            if (action.type == kActionNone) {
                break;
            }
            switch (action.type) {
                case kUpgradeOneCardInHand:
                {
                    // if valid target
                    if (hand.node_ptr->collection.card.size() > target) {
                        card_index_t card_index =
                            hand.node_ptr->collection.card[target].first;
                        // if upgraded version exists
                        if (card_map[card_index]->upgraded_version != nullptr) {
                            card_index_t new_index =
                                card_map[card_index]->upgraded_version->GetIndex();
                            hand.RemoveCard(card_index);
                            hand.AddCard(new_index);
                        }
                    }
                    break;
                }
                case kUpgradeAllCardsInHand:
                {
                    CardCollectionPtr new_hand;
                    for (auto & deck_item : hand) {
                        card_index_t card_index = deck_item.first;
                        if (card_map[card_index]->upgraded_version != nullptr) {
                            card_index = card_map[card_index]->upgraded_version->GetIndex();
                        }
                        new_hand.AddCard(card_index, deck_item.second);
                    }
                    hand = new_hand;
                    break;
                }
                case kActionAttackPerfectedStrike:
                case kActionAttackHeavyBlade:
                case kActionAttackBowlingBash:
                case kActionAttackBodySlam:
                case kActionAttackFiendFire:
                case kActionAttack:
                {
                    uint16_t amount = 0;
                    uint16_t count = 0;
                    if (buff[kBuffRage] > 0) {
                        block += buff[kBuffRage];
                    }
                    if (action.type == kActionAttack ||
                            action.type == kActionAttackIfPoisoned) {
                        amount = action.arg[0];
                        count = action.arg[1];
                    } else if (action.type == kActionAttackPerfectedStrike) {
                        amount = action.arg[0] + action.arg[1] * CountStrikeCards();
                        count = 1;
                    } else if (action.type == kActionAttackHeavyBlade) {
                        amount = action.arg[0];
                        count = 1;
                    } else if (action.type == kActionAttackBodySlam) {
                        amount = block;
                        count = 1;
                    } else if (action.type == kActionAttackBowlingBash) {
                        amount = action.arg[0];
                        count = 0;
                        for (auto & mob : monster) {
                            if (mob.Exists()) {
                                ++count;
                            }
                        }
                    } else if (action.type == kActionAttackFiendFire) {
                        // exhaust all cards
                        for (auto & deck_item : hand) {
                            const Card & card = *card_map[deck_item.first];
                            exhaust_pile.AddCard(deck_item.first, deck_item.second);
                            amount += action.arg[0] * deck_item.second;
                        }
                        count = 1;
                        hand.Clear();
                    } else {
                        printf("ERROR: unexpected attack type\n");
                        exit(1);
                    }
                    assert(card.flag.targeted);
                    if (action.type == kActionAttackHeavyBlade) {
                        amount += buff[kBuffStrength] * action.arg[1];
                    } else {
                        amount += buff[kBuffStrength];
                    }
                    if (relics.akabeko_active) {
                        relics.akabeko_active = 0;
                        amount += 8;
                    }
                    if (stance == kStanceWrath) {
                        amount *= 2;
                    }
                    if (buff[kBuffWeak]) {
                        amount = amount * 3 / 4;
                    }
                    if (action.type == kActionAttackBowlingBash) {
                        amount = amount * count;
                        count = 1;
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
                case kActionAttackAllWhirlwind:
                {
                    uint16_t amount = 0;
                    uint16_t count = 0;
                    if (buff[kBuffRage] > 0) {
                        block += buff[kBuffRage];
                    }
                    assert(!card.flag.targeted);
                    if (action.type == kActionAttackAll) {
                        amount = action.arg[0];
                        count = action.arg[1];
                    } else if (action.type == kActionAttackAllWhirlwind) {
                        amount = action.arg[0];
                        count = card_energy;
                    }
                    amount += buff[kBuffStrength];
                    if (relics.akabeko_active) {
                        relics.akabeko_active = 0;
                        amount += 8;
                    }
                    if (stance == kStanceWrath) {
                        amount *= 2;
                    }
                    if (buff[kBuffWeak]) {
                        amount = amount * 3 / 4;
                    }
                    for (int16_t i = 0; i < count; ++i) {
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
                    assert(card.flag.targeted);
                    if (mob.Exists()) {
                        mob.buff[action.arg[0]] += action.arg[1];
                    }
                    break;
                case kActionDebuffAll:
                {
                    assert(!card.flag.targeted);
                    for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
                        if (!monster[i].Exists()) {
                            continue;
                        }
                        monster[i].buff[action.arg[0]] += action.arg[1];
                    }
                    break;
                }
                case kActionLoseHP:
                {
                    TakeHPLoss(action.arg[0]);
                    if (hp == 0) {
                        return;
                    }
                    break;
                }
                case kActionAddCardToDrawPile:
                {
                    draw_pile.AddCard(
                        (card_index_t) action.arg[0],
                        (card_count_t) action.arg[1]);
                    break;
                }
                case kActionAddCardToDiscardPile:
                {
                    discard_pile.AddCard(
                        (card_index_t) action.arg[0],
                        (card_count_t) action.arg[1]);
                    break;
                }
                case kActionAddCardToHand:
                {
                    hand.AddCard(
                        (card_index_t) action.arg[0],
                        (card_count_t) action.arg[1]);
                    break;
                }
                case kActionChangeStance:
                {
                    StanceEnum new_stance = (StanceEnum) action.arg[0];
                    if (stance != action.arg[0]) {
                        if (stance == kStanceCalm) {
                            energy += 2;
                        }
                        stance = new_stance;
                        // move Flurry of Blows from discard to hand
                        card_index_t flurry_plus_index = card_flurry_of_blows.GetIndex();
                        card_count_t flurry_plus_count =
                            discard_pile.CountCard(flurry_plus_index);
                        if (flurry_plus_count > 0 && hand.Count() < MAX_HAND_SIZE) {
                            card_count_t to_add = flurry_plus_count;
                            if (hand.Count() + to_add > MAX_HAND_SIZE) {
                                to_add = MAX_HAND_SIZE - hand.Count();
                            }
                            assert(to_add > 0);
                            hand.AddCard(flurry_plus_count, to_add);
                            discard_pile.RemoveCard(flurry_plus_count, to_add);
                        }
                        card_index_t flurry_index = card_flurry_of_blows.GetIndex();
                        card_count_t flurry_count = discard_pile.CountCard(flurry_index);
                        if (flurry_count > 0 && hand.Count() < MAX_HAND_SIZE) {
                            card_count_t to_add = flurry_count;
                            if (hand.Count() + to_add > MAX_HAND_SIZE) {
                                to_add = MAX_HAND_SIZE - hand.Count();
                            }
                            assert(to_add > 0);
                            hand.AddCard(flurry_index, to_add);
                            discard_pile.RemoveCard(flurry_index, to_add);
                        }
                    }
                    break;
                }
                case kActionGainEnergy:
                {
                    energy += action.arg[0];
                    break;
                }
#ifdef USE_ORBS
                case kActionChannelOrb:
                {
                    for (int32_t i = 0; i < action.arg[1]; ++i) {
                        ChannelOrb((OrbEnum) action.arg[0]);
                    }
                    break;
                }
                case kActionEvokeOrb:
                {
                    if (!orbs.empty()) {
                        for (int32_t i = 0; i < action.arg[0]; ++i) {
                            EvokeOrb(orbs[0]);
                        }
                        PopOrb();
                    }
                    break;
                }
#endif
                case kActionScry:
                {
                    static bool first_time = true;
                    if (first_time) {
                        printf("WARNING: scry is not implemented, action ignored\n");
                        first_time = false;
                    }
                    break;
                }
                case kActionInWrath:
                {
                    if (stance != kStanceWrath) {
                        ++i;
                    }
                    break;
                }
                case kActionHeal:
                {
                    Heal(action.arg[0]);
                    break;
                }
                case kActionIfPoisoned:
                {
                    assert(last_card_attack_matters);
                    if (!mob.buff[kBuffPoison]) {
                        ++i;
                    }
                    break;
                }
                case kActionLastCardAttack:
                {
                    assert(last_card_attack_matters);
                    if (!flag.last_card_attack) {
                        ++i;
                    }
                    break;
                }
                case kActionLastCardSkill:
                {
                    assert(last_card_skill_matters);
                    if (!flag.last_card_skill) {
                        ++i;
                    }
                    break;
                }
                case kActionDrawCards:
                {
                    if (!buff[kBuffNoDraw]) {
                        assert(pending_action[MAX_PENDING_ACTIONS - 1].type == kActionNone);
                        for (auto & this_action : pending_action) {
                            if (this_action.type == kActionNone) {
                                this_action = action;
                                break;
                            }
                        }
                    }
                    break;
                }
                default:
                {
                    printf("ERROR: unexpected action type\n");
                    exit(1);
                }
            }
        }
        flag.last_card_attack = card.flag.attack;
        flag.last_card_skill = card.flag.skill;
    }
    // return true if this node is strictly worse or equal to the given node
    bool IsWorseOrEqual(const Node & that) const {
        if (that.IsBattleDone() &&
            objective <= that.objective) {
            return true;
        }
        if (IsBattleDone()) {
            return false;
        }
        // after this point, neither battle is complete
        for (std::size_t i = 0; i < MAX_PENDING_ACTIONS; ++i) {
            if (pending_action[i].type != that.pending_action[i].type) {
                return false;
            }
            if (pending_action[i].type == kActionNone) {
                break;
            }
            if (pending_action[i].arg[0] != that.pending_action[i].arg[0]) {
                return false;
            }
        }
        if (discard_pile != that.discard_pile) {
            return false;
        }
        if (draw_pile != that.draw_pile) {
            return false;
        }
        if (exhaust_pile != that.exhaust_pile) {
            return false;
        }
        if (hand != that.hand) {
            return false;
        }
        if (turn != that.turn) {
            return false;
        }
        if (last_card_attack_matters &&
                flag.last_card_attack != that.flag.last_card_attack) {
            return false;
        }
        if (last_card_skill_matters &&
                flag.last_card_skill != that.flag.last_card_skill) {
            return false;
        }
        if (stance != that.stance) {
            return false;
        }
        if (hp > that.hp) {
            return false;
        }
        if (block > that.block) {
            return false;
        }
        if (energy > that.energy) {
            return false;
        }
        //if (cards_to_draw != that.cards_to_draw) {
        //    return false;
        //}
        if (hand != that.hand) {
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
    // return true if this is a terminal node
    bool IsTerminal() const {
        return child.empty() && IsBattleDone();
    }
};

// output to a stringstream
std::stringstream & operator<< (std::stringstream & out, const Node & node) {
    out << node.ToString();
    return out;
}

bool Node::last_card_skill_matters = false;
bool Node::last_card_attack_matters = false;

// emtpy deck
CardCollectionPtr Node::deck = CardCollectionPtr();
