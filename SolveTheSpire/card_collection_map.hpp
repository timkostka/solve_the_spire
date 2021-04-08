#pragma once

#include <set>

#include "card.hpp"
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
    // default constructor
    CardCollectionPtr() : ptr(nullptr) {
    }
    // constructor from CardCollection
    CardCollectionPtr(const CardCollection & deck) :
        ptr(CardCollectionMap::Find(deck)) {
    }
    // clear the deck
    void Clear() {
        CardCollection empty_deck;
        ptr = CardCollectionMap::Find(empty_deck);
    }
    // add a card
    // return number of cards in pile
    uint16_t Count() const {
        return ptr->total;
    }
    // return true if pile is empty
    bool IsEmpty() const {
        return Count() == 0;
    }
    // remove a card
    void RemoveCard(uint16_t index, uint16_t count = 1) {
        CardCollection new_deck = *ptr;
        new_deck.RemoveCard(index, count);
        ptr = CardCollectionMap::Find(new_deck);
    }
    // add a card
    void AddCard(uint16_t index, uint16_t count = 1) {
        CardCollection new_deck = *ptr;
        new_deck.AddCard(index, count);
        ptr = CardCollectionMap::Find(new_deck);
    }
    // add a card
    void operator+= (const uint16_t index) {
        AddCard(index, 1);
    }
    // add a card
    void operator+= (const Card & c) {
        AddCard(c);
    }
    // add a card
    void AddCard(const Card & c, uint16_t count = 1) {
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
};
