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

//TEST(TestCards, TestDefaultAttackNode) {
//    Node this_node = GetDefaultAttackNode();
//    this_node.EndTurn();
//    ASSERT_EQ(this_node.hp, 100 - 10);
//}
//
//TEST(TestCards, TestMetallicize) {
//    Node this_node = GetDefaultAttackNode();
//    this_node.hand.AddCard(card_metallicize.GetIndex());
//    this_node.PlayCard(card_metallicize.GetIndex());
//    this_node.EndTurn();
//    ASSERT_EQ(this_node.hp, 100 - 10 + 3);
//}
//
//TEST(TestCards, TestWhirlwind) {
//    Node this_node = GetDefaultAttackNode();
//    ASSERT_EQ(this_node.energy, 3);
//    AddAndPlayCard(card_whirlwind, this_node);
//    ASSERT_EQ(this_node.monster[0].hp, 100 - 5 * 3);
//    ASSERT_EQ(this_node.energy, 0);
//}
//
//TEST(TestCards, TestRage) {
//    Node this_node = GetDefaultAttackNode();
//    AddAndPlayCard(card_rage, this_node);
//    ASSERT_EQ(this_node.block, 0);
//    AddAndPlayCard(card_strike, this_node);
//    ASSERT_EQ(this_node.block, 3);
//    AddAndPlayCard(card_cleave, this_node);
//    ASSERT_EQ(this_node.block, 6);
//}
//
//TEST(TestDecks, TestUpgrades) {
//    CardCollectionPtr unupgraded_deck;
//    CardCollectionPtr upgraded_deck;
//    unupgraded_deck.AddCard(card_strike);
//    upgraded_deck.AddCard(card_strike_plus);
//    ASSERT_TRUE(unupgraded_deck.IsWorseOrEqual(upgraded_deck));
//    ASSERT_FALSE(upgraded_deck.IsWorseOrEqual(unupgraded_deck));
//    ASSERT_TRUE(unupgraded_deck.IsWorseOrEqual(unupgraded_deck));
//    ASSERT_TRUE(upgraded_deck.IsWorseOrEqual(upgraded_deck));
//}
//
//// ensure that the solver selects the path that does the most damage
//// even if death is inevitable
//TEST(TestSolver, TestDoMaxDamageOnDeath) {
//    Node this_node = GetDefaultAttackNode();
//    this_node.hp = 10;
//    TreeStruct tree(this_node);
//    tree.Expand();
//    ASSERT_DOUBLE_EQ(tree.death_chance, 1.0);
//    ASSERT_DOUBLE_EQ(tree.final_hp, 0.0);
//    ASSERT_DOUBLE_EQ(tree.remaining_mob_hp, 94.0);
//}
//
//// ensure that the solver selects the path that does the most damage
//// even if death is inevitable
//TEST(TestSolver, TestDoMaxDamageOnDeath2) {
//    Node this_node = GetDefaultAttackNode();
//    this_node.hp = 30;
//    TreeStruct tree(this_node);
//    tree.Expand();
//    ASSERT_DOUBLE_EQ(tree.death_chance, 1.0);
//    ASSERT_DOUBLE_EQ(tree.final_hp, 0.0);
//    ASSERT_DOUBLE_EQ(tree.remaining_mob_hp, 100 - 6 * 3);
//}

//// ensure that the solver selects the path that does the most damage
//// even if death is inevitable
//TEST(TestSolver, TestHemoSuicide) {
//    Node this_node = GetDefaultAttackNode();
//    this_node.hp = 2;
//    auto & mob = this_node.monster[0];
//    mob.base = &base_mob_gremlin_nob;
//    mob.hp = 13;
//    mob.max_hp = 80;
//    mob.last_intent[0] = 1;
//    mob.buff[kBuffStrength] = 15;
//    mob.buff[kBuffEnrage] = 3;
//    this_node.hand.Clear();
//    this_node.hand.AddCard(card_hemokinesis);
//    this_node.hand.AddCard(card_strike);
//    this_node.hand.AddCard(card_bash);
//    this_node.hand.AddCard(card_defend, 2);
//    TreeStruct tree(this_node);
//    tree.Expand();
//    this_node.PrintTree();
//    ASSERT_DOUBLE_EQ(tree.death_chance, 0.0);
//    ASSERT_DOUBLE_EQ(tree.final_hp, 2.0);
//    ASSERT_DOUBLE_EQ(tree.remaining_mob_hp, 0);
//}

//// Game(maxobj=24, turn=6, p=6.4e-05, hp=18/80, energy=3, hand={5 cards: Strike, 3xDefend, Hemokinesis}, mob0=(Gremlin Nob, 11hp, Rush, 3xStr, 1xVuln, 3xEnrage))
//TEST(TestSolver, TestFinishWithHemo) {
//    Node this_node = GetDefaultAttackNode();
//    this_node.hp = 18;
//    this_node.relics.burning_blood = 1;
//    auto & mob = this_node.monster[0];
//    mob.base = &base_mob_gremlin_nob;
//    mob.hp = 11;
//    mob.max_hp = 80;
//    mob.last_intent[0] = 1;
//    mob.buff[kBuffStrength] = 3;
//    mob.buff[kBuffVulnerable] = 1;
//    mob.buff[kBuffEnrage] = 3;
//    this_node.hand.Clear();
//    this_node.hand.AddCard(card_strike);
//    this_node.hand.AddCard(card_defend, 3);
//    this_node.hand.AddCard(card_hemokinesis);
//    TreeStruct tree(this_node);
//    tree.Expand();
//    this_node.PrintTree();
//    ASSERT_DOUBLE_EQ(tree.death_chance, 0.0);
//    ASSERT_DOUBLE_EQ(tree.final_hp, 2.0);
//    ASSERT_DOUBLE_EQ(tree.remaining_mob_hp, 0);
//}

// test for goblin nob fight bug
// Game(solved, obj=32.3197, turn=1, p=0.0433, hp=72/80, hand={5 cards: 4xStrike, Defend}, mob0=(Gremlin Nob, 87hp))
TEST(TestSolver, TestNobFightBug) {
    Node this_node = GetDefaultAttackNode();
    this_node.hp = 72;
    this_node.max_hp = 80;
    this_node.relics.burning_blood = 1;
    auto & mob = this_node.monster[0];
    mob.base = &base_mob_gremlin_nob;
    mob.hp = 87;
    mob.max_hp = 87;
    mob.last_intent[0] = 0;
    this_node.hand.Clear();
    this_node.hand.AddCard(card_strike, 4);
    this_node.hand.AddCard(card_defend, 1);
    TreeStruct tree(this_node);
    tree.Expand();
    this_node.PrintTree();
}
