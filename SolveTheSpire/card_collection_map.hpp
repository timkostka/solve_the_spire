#pragma once

#include <set>

struct CardCollection;

struct CardCollectionMap {
    // set of all current card collections
    static std::set<CardCollection> collection;
    // return a pointer to this deck in the collection
    // if it doesn't exist, add it to the collection
    static const CardCollection * Find(const CardCollection & deck) {
        return &*collection.insert(deck).first;
    }
};
