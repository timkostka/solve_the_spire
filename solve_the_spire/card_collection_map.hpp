#pragma once

#include <set>
#include <vector>

#include "defines.h"
#include "cards.hpp"
#include "card_collection.hpp"

// holds a deck along with how to get to other nearby decks
struct CardCollectionNode {
    // card collection itself
    CardCollection collection;
    // new card collection node if we add a given card index
    mutable std::vector<const CardCollectionNode *> add_card_node;
    // new card collection node if we add a remove a card index
    mutable std::vector<const CardCollectionNode *> remove_card_node;
    // comparison
    bool operator< (const CardCollectionNode & that) const {
        return collection < that.collection;
    };
};

// card collection pointer acts like a card collection but with massive optimizations
// for comparing and adding/removing cards
struct CardCollectionPtr {
    // set of all current card collections
    static std::set<CardCollectionNode> deck_collection;
    // empty card collection
    static CardCollectionNode empty_node;
    // pointer to card collection node
    const CardCollectionNode * node_ptr;
    // clear the deck
    void Clear() {
        node_ptr = &empty_node;
    }
    // default constructor
    CardCollectionPtr() : node_ptr(&empty_node) {
    }
    // comparison
    bool operator == (const CardCollectionPtr & that) const {
        return node_ptr == that.node_ptr;
    }
    // inequality comparision
    bool operator != (const CardCollectionPtr & that) const {
        return !(*this == that);
    }
    // add a card
    // return number of cards in pile
    card_count_t Count() const {
        return node_ptr->collection.total;
    }
    // return number of the given card in the pile
    card_count_t CountCard(card_index_t card_index) const {
        return node_ptr->collection.CountCard(card_index);
    }
    // return true if pile is empty
    bool IsEmpty() const {
        return node_ptr == &empty_node;
    }
    // add a card
    void AddCard(card_index_t index) {
        // see if it's calculated already
        if (node_ptr->add_card_node.size() > index) {
            if (node_ptr->add_card_node[index] != nullptr) {
                node_ptr = node_ptr->add_card_node[index];
                return;
            }
        } else {
            node_ptr->add_card_node.resize(index + 1, nullptr);
        }
        // create new collection
        CardCollectionNode node = {
            node_ptr->collection,
            std::vector<const CardCollectionNode *>(),
            std::vector<const CardCollectionNode *>()};
        node.collection.AddCard(index);
        // see if it exists
        auto it = deck_collection.find(node);
        if (it == deck_collection.end()) {
            // doesn't exist, so create a new entry and add it
            auto result = deck_collection.insert(node);
            const CardCollectionNode * new_node_ptr = &*result.first;
            node_ptr->add_card_node[index] = new_node_ptr;
            new_node_ptr->remove_card_node.resize(index + 1, nullptr);
            new_node_ptr->remove_card_node[index] = node_ptr;
            node_ptr = new_node_ptr;
        } else {
            // already exists, so just update add/remove pointers
            const CardCollectionNode * new_node_ptr = &*it;
            node_ptr->add_card_node[index] = new_node_ptr;
            if (new_node_ptr->remove_card_node.size() <= index) {
                new_node_ptr->remove_card_node.resize(index + 1, nullptr);
            }
            new_node_ptr->remove_card_node[index] = node_ptr;
            node_ptr = new_node_ptr;
        }
    }
    void AddCard(card_index_t index, card_count_t count) {
        for (card_count_t i = 0; i < count; ++i) {
            AddCard(index);
        }
    }
    // add a card
    void AddCard(const Card & card, card_count_t count = 1) {
        AddCard(card.GetIndex(), count);
    }
    // add a card
    void AddCard(const deck_item_t & item) {
        AddCard(item.first, item.second);
    }
    // add a card
    void RemoveCard(card_index_t index) {
        // if only one card, set it to the empty deck
        assert(CountCard(index) > 0);
        if (node_ptr->collection.total == 1) {
            node_ptr = &empty_node;
            return;
        }
        // see if it's calculated already
        if (node_ptr->remove_card_node.size() > index) {
            if (node_ptr->remove_card_node[index] != nullptr) {
                node_ptr = node_ptr->remove_card_node[index];
                return;
            }
        } else {
            node_ptr->remove_card_node.resize(index + 1, nullptr);
        }
        // create new collection
        CardCollectionNode node = {
            node_ptr->collection,
            std::vector<const CardCollectionNode *>(),
            std::vector<const CardCollectionNode *>()};
        node.collection.RemoveCard(index);
        // see if it exists
        auto it = deck_collection.find(node);
        if (it == deck_collection.end()) {
            // doesn't exist, so create a new entry and add it
            auto result = deck_collection.insert(node);
            const CardCollectionNode * new_node_ptr = &*result.first;
            node_ptr->remove_card_node[index] = new_node_ptr;
            new_node_ptr->add_card_node.resize(index + 1, nullptr);
            new_node_ptr->add_card_node[index] = node_ptr;
            node_ptr = new_node_ptr;
        } else {
            // already exists, so just update add/remove pointers
            const CardCollectionNode * new_node_ptr = &*it;
            node_ptr->remove_card_node[index] = new_node_ptr;
            if (new_node_ptr->add_card_node.size() <= index) {
                new_node_ptr->add_card_node.resize(index + 1, nullptr);
            }
            new_node_ptr->add_card_node[index] = node_ptr;
            node_ptr = new_node_ptr;
        }
    }
    void RemoveCard(card_index_t index, card_count_t count) {
        for (card_count_t i = 0; i < count; ++i) {
            RemoveCard(index);
        }
    }
    // add a card
    void RemoveCard(const Card & card, card_count_t count = 1) {
        RemoveCard(card.GetIndex(), count);
    }
    // add a card
    void RemoveCard(const deck_item_t & item) {
        RemoveCard(item.first, item.second);
    }
    // add an entire deck
    void AddDeck(const CardCollectionPtr & that) {
        // if this is empty, just set the pointer to the other deck
        if (IsEmpty()) {
            *this = that;
            return;
        }
        // else add each card individually
        for (auto & deck_item : that) {
            AddCard(deck_item);
        }
    }
    // add an entire deck
    // TODO: remove this after change to Select is done
    void AddDeck(const CardCollection & that) {
        // add each card individually
        for (auto & deck_item : that.card) {
            AddCard(deck_item);
        }
    }
    // set equal to a card collection
    // TODO: remove this after change to Select is done
    void operator= (const CardCollection & that) {
        Clear();
        AddDeck(that);
    }
    // range iterator loops through cards in deck
    std::vector<deck_item_t>::const_iterator begin() const {
        return node_ptr->collection.card.begin();
    }
    // range iterator loops through cards in deck
    std::vector<deck_item_t>::const_iterator end() const {
        return node_ptr->collection.card.end();
    }
    // convert to a string
    std::string ToString() const {
        return node_ptr->collection.ToString();
    }
    // return the local index of the given card index
    card_index_t GetLocalIndex(card_index_t index) const {
        for (card_index_t i = 0; i < node_ptr->collection.card.size(); ++i) {
            if (node_ptr->collection.card[i].first == index) {
                return i;
            }
        }
        printf("ERROR: could not find card\n");
        exit(1);
    }
    // return true if this deck is strictly worse or equal to another deck
    bool IsWorseOrEqual(const CardCollectionPtr & that) const {
        // if they're equal
        if (this == &that) {
            return true;
        }
        // if we don't consider upgraded cards strictly better, they're different
        if (!upgrades_strictly_better) {
            return false;
        }
        // if card number is different, neither is worse than the other
        if (!upgrades_strictly_better ||
                node_ptr->collection.total != that.node_ptr->collection.total) {
            return false;
        }
        // if this deck contains more upgraded cards, it is not worse
        for (auto & item : that.node_ptr->collection.card) {
            // count number of this card or upgraded cards in current deck
            card_index_t card_index = item.first;
            const Card & card = *card_map[card_index];
            card_count_t that_count = item.second;
            card_count_t this_count = node_ptr->collection.CountCard(card_index);
            card_count_t this_upgraded_count = 0;
            card_count_t that_upgraded_count = 0;
            if (card.upgraded_version != nullptr) {
                card_index_t upgraded_index = card.upgraded_version->GetIndex();
                this_upgraded_count = node_ptr->collection.CountCard(upgraded_index);
                that_upgraded_count = that.node_ptr->collection.CountCard(upgraded_index);
            }
            // this has more upgraded cards and is potentially better
            if (this_upgraded_count > that_upgraded_count) {
                return false;
            }
            // this has more cards and is potentially better
            if (this_count + this_upgraded_count > that_count + that_upgraded_count) {
                return false;
            }
        }
        return true;
    }
};

// empty card collection
CardCollectionNode CardCollectionPtr::empty_node;

// set of all current card collections
std::set<CardCollectionNode> CardCollectionPtr::deck_collection;
