#include "gtest/gtest.h"

#include "node.hpp"
#include "tree.hpp"

Node GetDefaultAttackNode() {
    Node node;
    node.hp = 100;
    node.max_hp = 100;
    node.relics = {0};
    node.InitializeStartingNode();
    node.PopPendingAction();
    node.PopPendingAction();
    node.hand.AddCard(card_strike);
    node.turn = 1;
    node.energy = 3;
    node.monster[0] = base_mob_test_100hp_10hp_attacker;
    node.monster[0].last_intent[0] = 0;
    return node;
}

// add a card and play it
void AddAndPlayCard(const Card & card, Node & node, uint8_t target = 0) {
    node.hand.AddCard(card.GetIndex());
    node.PlayCard(card.GetIndex(), target);
}

TEST(TestCards, TestDefaultAttackNode) {
    Node this_node = GetDefaultAttackNode();
    this_node.EndTurn();
    ASSERT_EQ(this_node.hp, 100 - 10);
}

TEST(TestCards, TestMetallicize) {
    Node this_node = GetDefaultAttackNode();
    this_node.hand.AddCard(card_metallicize.GetIndex());
    this_node.PlayCard(card_metallicize.GetIndex());
    this_node.EndTurn();
    ASSERT_EQ(this_node.hp, 100 - 10 + 3);
}

TEST(TestCards, TestWhirlwind) {
    Node this_node = GetDefaultAttackNode();
    ASSERT_EQ(this_node.energy, 3);
    AddAndPlayCard(card_whirlwind, this_node);
    ASSERT_EQ(this_node.monster[0].hp, 100 - 5 * 3);
    ASSERT_EQ(this_node.energy, 0);
}

TEST(TestCards, TestRage) {
    Node this_node = GetDefaultAttackNode();
    AddAndPlayCard(card_rage, this_node);
    ASSERT_EQ(this_node.block, 0);
    AddAndPlayCard(card_strike, this_node);
    ASSERT_EQ(this_node.block, 3);
    AddAndPlayCard(card_cleave, this_node);
    ASSERT_EQ(this_node.block, 6);
}

TEST(TestDecks, TestUpgrades) {
    CardCollectionPtr unupgraded_deck;
    CardCollectionPtr upgraded_deck;
    unupgraded_deck.AddCard(card_strike);
    upgraded_deck.AddCard(card_strike_plus);
    ASSERT_TRUE(unupgraded_deck.IsWorseOrEqual(upgraded_deck));
    ASSERT_FALSE(upgraded_deck.IsWorseOrEqual(unupgraded_deck));
    ASSERT_TRUE(unupgraded_deck.IsWorseOrEqual(unupgraded_deck));
    ASSERT_TRUE(upgraded_deck.IsWorseOrEqual(upgraded_deck));
}

// ensure that the solver selects the path that does the most damage
// even if death is inevitable
TEST(TestSolver, TestDoMaxDamageOnDeath) {
    Node this_node = GetDefaultAttackNode();
    this_node.hp = 10;
    TreeStruct tree(this_node);
    tree.Expand();
    ASSERT_DOUBLE_EQ(tree.death_chance, 1.0);
    ASSERT_DOUBLE_EQ(tree.final_hp, 0.0);
    ASSERT_DOUBLE_EQ(tree.remaining_mob_hp, 94.0);
}

// ensure that the solver selects the path that does the most damage
// even if death is inevitable
TEST(TestSolver, TestDoMaxDamageOnDeath2) {
    Node this_node = GetDefaultAttackNode();
    this_node.hp = 30;
    TreeStruct tree(this_node);
    tree.Expand();
    ASSERT_DOUBLE_EQ(tree.death_chance, 1.0);
    ASSERT_DOUBLE_EQ(tree.final_hp, 0.0);
    ASSERT_DOUBLE_EQ(tree.remaining_mob_hp, 100 - 6 * 3);
}

// ensure that the solver selects the path that does the most damage
// even if death is inevitable
TEST(TestSolver, TestHemoSuicide) {
    Node this_node = GetDefaultAttackNode();
    this_node.hp = 2;
    auto & mob = this_node.monster[0];
    mob.base = &base_mob_gremlin_nob;
    mob.hp = 13;
    mob.max_hp = 80;
    mob.last_intent[0] = 1;
    mob.buff[kBuffStrength] = 15;
    mob.buff[kBuffEnrage] = 3;
    this_node.hand.Clear();
    this_node.hand.AddCard(card_hemokinesis);
    this_node.hand.AddCard(card_strike);
    this_node.hand.AddCard(card_bash);
    this_node.hand.AddCard(card_defend, 2);
    TreeStruct tree(this_node);
    tree.Expand();
    ASSERT_DOUBLE_EQ(tree.death_chance, 0.0);
    ASSERT_DOUBLE_EQ(tree.final_hp, 2.0);
    ASSERT_DOUBLE_EQ(tree.remaining_mob_hp, 0);
}

// ensure offering is not used if not needed
TEST(TestSolver, TestOffering1) {
    Node this_node = GetDefaultAttackNode();
    auto & mob = this_node.monster[0];
    mob.hp = 12;
    this_node.hand.Clear();
    this_node.hand.AddCard(card_offering);
    this_node.hand.AddCard(card_strike, 2);
    this_node.draw_pile.AddCard(card_wound, 5);
    TreeStruct tree(this_node);
    tree.Expand();
    this_node.PrintTree();
    ASSERT_DOUBLE_EQ(tree.final_hp, 100.0);
}

// ensure offering is used if needed
TEST(TestSolver, TestOffering2) {
    Node this_node = GetDefaultAttackNode();
    auto & mob = this_node.monster[0];
    mob.hp = 12;
    this_node.hand.Clear();
    this_node.hand.AddCard(card_offering);
    this_node.hand.AddCard(card_wound, 2);
    this_node.draw_pile.AddCard(card_strike, 5);
    TreeStruct tree(this_node);
    tree.Expand();
    this_node.PrintTree();
    ASSERT_DOUBLE_EQ(tree.final_hp, 94.0);
}
