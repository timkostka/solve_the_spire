#pragma once

#include <set>

#include "defines.h"
#include "cards.hpp"
#include "card_collection.hpp"

struct CardCollectionMap {
    // set of all current card collections
    static std::set<CardCollection> collection;
    // return a pointer to this deck in the collection
    // if it doesn't exist, add it to the collection
    static const CardCollection * Find(const CardCollection & deck) {
        return &*collection.insert(deck).first;
    }
};

struct CardCollectionPtr {
    // pointer to card collection within CardCollectionMap.collection
    const CardCollection * ptr;
    // clear the deck
    void Clear() {
        CardCollection empty_deck;
        ptr = CardCollectionMap::Find(empty_deck);
    }
    // default constructor
    CardCollectionPtr() : ptr(nullptr) {
        Clear();
    }
    // constructor from CardCollection
    CardCollectionPtr(const CardCollection & deck) :
        ptr(CardCollectionMap::Find(deck)) {
    }
    // compare two
    bool operator == (const CardCollectionPtr & that) const {
        return ptr == that.ptr;
    }
    bool operator != (const CardCollectionPtr & that) const {
        return !(*this == that);
    }
    // add a card
    // return number of cards in pile
    card_count_t Count() const {
        return ptr->total;
    }
    // return number of the given card in the pile
    card_count_t CountCard(card_index_t card_index) const {
        return ptr->CountCard(card_index);
    }
    // return true if pile is empty
    bool IsEmpty() const {
        return Count() == 0;
    }
    // remove a card
    void RemoveCard(card_index_t index, card_count_t count = 1) {
        CardCollection new_deck = *ptr;
        new_deck.RemoveCard(index, count);
        ptr = CardCollectionMap::Find(new_deck);
    }
    // add a card
    void AddCard(card_index_t index, card_count_t count = 1) {
        CardCollection new_deck = *ptr;
        new_deck.AddCard(index, count);
        ptr = CardCollectionMap::Find(new_deck);
    }
    // add a card
    void operator+= (const card_index_t index) {
        AddCard(index, 1);
    }
    // add a card
    void operator+= (const Card & c) {
        AddCard(c);
    }
    // add a card
    void AddCard(const Card & c, card_count_t count = 1) {
        AddCard(c.GetIndex(), count);
    }
    // add a card collection
    void AddCard(const CardCollection & that) {
        for (auto & c : that.card) {
            AddCard(c.first, c.second);
        }
    }
    // add a card
    void operator+= (const CardCollection & that) {
        AddCard(that);
    }
    // add a card
    void operator+= (const CardCollectionPtr & that) {
        *this += *that.ptr;
    }
    // convert to a string
    std::string ToString() const {
        return ptr->ToString();
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
        if (!upgrades_strictly_better || ptr->total != that.ptr->total) {
            return false;
        }
        // if this deck contains more upgraded cards, it is not worse
        for (auto & item : that.ptr->card) {
            // count number of this card or upgraded cards in current deck
            card_index_t card_index = item.first;
            const Card & card = *card_map[card_index];
            card_count_t that_count = item.second;
            card_count_t this_count = ptr->CountCard(card_index);
            card_count_t this_upgraded_count = 0;
            card_count_t that_upgraded_count = 0;
            if (card.upgraded_version != nullptr) {
                card_index_t upgraded_index = card.upgraded_version->GetIndex();
                this_upgraded_count = ptr->CountCard(upgraded_index);
                that_upgraded_count = that.ptr->CountCard(upgraded_index);
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

// set of all current card collections
std::set<CardCollection> CardCollectionMap::collection;
