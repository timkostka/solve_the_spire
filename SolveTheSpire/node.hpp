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
#include "orbs.hpp"

// map of decks
// Each node stores a pointer to a deck within this collection rather than
// storing the entire collection.  This reduces the number of std::vectors
// within the Node along with the associated create/copy/delete operations.
//CardCollectionMapStruct card_collections;

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

// A Node contains all information about a game node.
struct Node {
    static bool last_card_skill_matters;
    static bool last_card_attack_matters;
    // fight type
    FightEnum fight_type;
    // node number (to help with sorting order)
    std::size_t index;
    // turn number
    uint16_t turn;
    // layer number (for evaluating tree)
    // increase by 1 for each decision that happens so that we can evaluate
    // the lowest layer to keep memory requirements low
    uint16_t layer;
    // max player HP
    uint16_t max_hp;
    // stance
    StanceEnum stance;
    // relic state
    RelicStruct relics;
    // orb slots
    uint8_t orb_slots;
    // focus
    uint8_t focus;
    // orbs
    std::vector<OrbStruct> orbs;
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
    // true if last card was an attack
    bool last_card_attack;
    // true if last card was a skill
    bool last_card_skill;
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
    // return an objective function used to evaluate the best decision
    // this function may only use information within this node--no information
    // from children is allowed
    double GetPathObjective() const {
        double x = 5.0 * hp;
        for (int i = 0; i < MAX_MOBS_PER_NODE; ++i) {
            if (monster[i].Exists()) {
                x += monster[i].max_hp - monster[i].hp;
            }
        }
        // favor evaluating later turns
        x += layer * 1000;
        //x += turn * 1000;
        return x;
    }
    // return best possible ultimate objective function for a child of this node
    // (it's okay to overestimate, but not ideal)
    double GetMaxFinalObjective() const {
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
        tree_solved = true;
        composite_objective = hp;
        // TODO: if we die, ensure we choose path that lowers the mob HP the most
        //if (hp == 0) {
        //    for (auto & mob : monster) {
        //        if (mob.Exists()) {
        //            composite_objective -= mob.hp / 1000.0;
        //        }
        //    }
        //}
    }
    // initialize a start of fight node
    // (only things that need initialized outside of this are:
    // hp, max_hp, fight_type, deck
    void InitializeStartingNode() {
        assert(hp > 0);
        assert(max_hp >= hp);
        assert(!deck.IsEmpty());

        // TODO: populate last_card_attack/skill_matters based on cards in deck

        stance = kStanceNone;
        discard_pile.Clear();
        hand.Clear();
        exhaust_pile.Clear();
        // TODO: is deck shuffled at start of battle?
        //draw_pile.Clear();
        draw_pile = deck;
        last_card_attack = false;
        last_card_skill = false;

        focus = 0;
        index = 0;
        layer = 0;
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
        battle_done = false;
        composite_objective = GetMaxFinalObjective();
        path_objective = GetPathObjective();
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
            for (std::size_t i = 1; i < child.size(); ++i) {
                if (child[i]->composite_objective > max_objective) {
                    max_objective = child[i]->composite_objective;
                }
            }
            return max_objective;
        } else {
            // if not player choice, then return probability adjusted composite
            double total_probability = child[0]->probability;
            double objective = child[0]->probability * child[0]->composite_objective;
            double max_objective = child[0]->composite_objective;
            double min_objective = max_objective;
            for (std::size_t i = 1; i < child.size(); ++i) {
                total_probability += child[i]->probability;
                objective += child[i]->probability * child[i]->composite_objective;
                if (child[i]->composite_objective > max_objective) {
                    max_objective = child[i]->composite_objective;
                } else if (child[i]->composite_objective < min_objective) {
                    min_objective = child[i]->composite_objective;
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
    void CalculateCompositeObjectiveIncludingChildren() {
        for (auto & child_ptr : child) {
            child_ptr->CalculateCompositeObjectiveIncludingChildren();
        }
        composite_objective = CalculateCompositeObjective();
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
    // finish battle in which player is still alive
    void FinishBattle() {
        assert(!battle_done);
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
        turn = 1;
        ResetEnergy();
        cards_to_draw = 5;
        if (relics.pure_water) {
            hand.AddCard(card_miracle);
        }
        if (relics.ring_of_the_snake) {
            cards_to_draw += 2;
        }
        if (relics.cracked_core) {
            ChannelOrb(kOrbLightning);
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
        ss.precision(3);
        if (tree_solved) {
            ss << "solved, ";
            ss << "obj=" << composite_objective;
            first_item = false;
        } else {
            ss << "maxobj=" << composite_objective;
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
        ss << "hp=" << hp << "/" << max_hp;
        if (stance == kStanceWrath) {
            ss << ", Wrath";
        } else if (stance == kStanceCalm) {
            ss << ", Calm";
        }
        if (turn == 0) {
            ss << ", deck=" << deck.ToString();
            ss << ", fight=" << fight_map[fight_type].name;
        }
        if (turn == 0) {
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
        } else if (!hand.IsEmpty()) {
            ss << ", hand=" << hand.ToString();
        }
        if (!orbs.empty()) {
            ss << ", orbs=";
            ss << orbs[0].ToString();
            for (int i = 1; i < orbs.size(); ++i) {
                ss << "," << orbs[i].ToString();
            }
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
                this_indent += "\xc0\xc4"; // "+-";
                this_hanging_indent += "  ";
            } else {
                this_indent += "\xc3\xc4";
                this_hanging_indent += "\xb3 ";
            }
            this_child.PrintTree(collapse, highlight, this_indent, this_hanging_indent);
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
        // process orbs
        ProcessOrbsEndTurn();
        if (!MobsAlive()) {
            FinishBattle();
            return;
        }
        // exhaust all ethereal cards
        for (std::size_t i = 0; i < hand.ptr->card.size(); ++i) {
            const Card & card = *card_map[hand.ptr->card[i].first];
            if (card.flag.ethereal) {
                exhaust_pile.AddCard(
                    hand.ptr->card[i].first,
                    hand.ptr->card[i].second);
                hand.RemoveCard(
                    hand.ptr->card[i].first,
                    hand.ptr->card[i].second);
                --i;
            }
        }
        // discard all remaining cards except those we retain
        if (relics.runic_pyramid == 0) {
            CardCollectionPtr new_hand;
            new_hand.Clear();
            for (auto & item : hand.ptr->card) {
                const Card & card = *card_map[item.first];
                if (card.flag.retain) {
                    new_hand.AddCard(item.first, item.second);
                } else {
                    discard_pile.AddCard(item.first, item.second);
                }
            }
            // remove them from hand
            hand = new_hand;
        }
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
    // return the number of "Strike" cards present
    uint16_t CountStrikeCards() {
        uint16_t count = 0;
        for (auto & card : draw_pile.ptr->card) {
            if (card_map[card.first]->flag.strike) {
                count += card.second;
            }
        }
        for (auto & card : hand.ptr->card) {
            if (card_map[card.first]->flag.strike) {
                count += card.second;
            }
        }
        for (auto & card : discard_pile.ptr->card) {
            if (card_map[card.first]->flag.strike) {
                count += card.second;
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
    void PlayCard(uint16_t index, uint8_t target = 0) {
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
        energy -= card.cost;
        auto & mob = monster[target];
        if (card.flag.targeted) {
            assert(mob.Exists());
        }
        // do actions
        for (uint32_t i = 0; i < MAX_CARD_ACTIONS; ++i) {
            const auto & action = card.action[i];
            if (action.type == kActionNone) {
                break;
            }
            switch (action.type) {
                case kActionAttackPerfectedStrike:
                case kActionAttackHeavyBlade:
                case kActionAttackBowlingBash:
                case kActionAttack:
                {
                    uint16_t amount = 0;
                    uint16_t count = 0;
                    if (action.type == kActionAttack) {
                        amount = action.arg[0];
                        count = action.arg[1];
                    } else if (action.type == kActionAttackPerfectedStrike) {
                        amount = action.arg[0] + action.arg[1] * CountStrikeCards();
                        count = 1;
                    } else if (action.type == kActionAttackHeavyBlade) {
                        amount = action.arg[0];
                        count = 1;
                    } else if (action.type == kActionAttackBowlingBash) {
                        amount = action.arg[0];
                        count = 0;
                        for (auto & mob : monster) {
                            if (mob.Exists()) {
                                ++count;
                            }
                        }
                    } else {
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
                {
                    assert(!card.flag.targeted);
                    uint16_t amount = action.arg[0];
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
                case kActionAddCardToDrawPile:
                {
                    draw_pile.AddCard(action.arg[0]);
                    break;
                }
                case kActionAddCardToDiscardPile:
                {
                    discard_pile.AddCard(action.arg[0]);
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
                        uint16_t flurry_plus_index = card_flurry_of_blows.GetIndex();
                        uint16_t flurry_plus_count =
                            discard_pile.CountCard(flurry_plus_index);
                        if (flurry_plus_count > 0 && hand.Count() < MAX_HAND_SIZE) {
                            uint16_t to_add = flurry_plus_count;
                            if (hand.Count() + to_add > MAX_HAND_SIZE) {
                                to_add = MAX_HAND_SIZE - hand.Count();
                            }
                            assert(to_add > 0);
                            hand.AddCard(flurry_plus_count, to_add);
                            discard_pile.RemoveCard(flurry_plus_count, to_add);
                        }
                        uint16_t flurry_index = card_flurry_of_blows.GetIndex();
                        uint16_t flurry_count = discard_pile.CountCard(flurry_index);
                        if (flurry_count > 0 && hand.Count() < MAX_HAND_SIZE) {
                            uint16_t to_add = flurry_count;
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
                case kActionLastCardAttack:
                {
                    assert(last_card_attack_matters);
                    if (!last_card_attack) {
                        ++i;
                    }
                    break;
                }
                case kActionLastCardSkill:
                {
                    assert(last_card_skill_matters);
                    if (!last_card_skill) {
                        ++i;
                    }
                    break;
                }
                default:
                {
                    printf("ERROR: unexpected action type\n");
                    exit(1);
                    assert(false);
                }
            }
        }
        last_card_attack = card.flag.attack;
        last_card_skill = card.flag.skill;
    }
    // return true if this node is strictly worse or equal to the given node
    bool IsWorseOrEqual(const Node & that) const {
        if (last_card_attack_matters &&
                last_card_attack != that.last_card_attack) {
            return false;
        }
        if (last_card_skill_matters &&
                last_card_skill != that.last_card_skill) {
            return false;
        }
        if (stance != that.stance) {
            return false;
        }
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
