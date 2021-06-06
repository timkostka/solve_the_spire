#include "gtest/gtest.h"

#include "node.hpp"

Node GetDefaultAttackNode() {
    Node node;
    node.hp = 100;
    node.max_hp = 100;
    node.relics = {0};
    node.deck.Clear();
    node.deck.AddCard(card_strike);
    node.InitializeStartingNode();
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
