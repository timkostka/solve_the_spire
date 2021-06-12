#pragma once

#include <cmath>
#include <utility>
#include <cassert>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>
#include <iterator>

#include "cards.hpp"

using std::vector;
using std::string;

// type to hold card index
typedef uint8_t card_index_t;

// type to hold the max number of cards
typedef uint8_t card_count_t;

// card pair
typedef std::pair<card_index_t, card_count_t> deck_item_t;


double factorial(double x) {
    double result = 1;
    while (x > 1) {
        result *= x;
        --x;
    }
    return result;
}

double ncr(double n, double r) {
    return factorial(n) / factorial(r) / factorial(n - r);
}


// a card collection is a collection of cards
struct CardCollection {
    // total number of cards
    card_count_t total;
    // list of cards and the count of each
    std::vector<deck_item_t> card;
    // constructor
    CardCollection() : total(0) {
    }
    // sorting operator
    bool operator< (const CardCollection & that) const {
        return card < that.card;
    }
    // return number of the given card in the pile
    card_count_t CountCard(card_index_t card_index) const {
        for (auto & this_card : card) {
            if (this_card.first == card_index) {
                return this_card.second;
            }
        }
        return 0;
    }
    // return number of cards in pile
    card_count_t Count() const {
        return total;
    }
    // return true if pile is empty
    bool IsEmpty() const {
        return total == 0;
    }
    // delete all cards
    void Clear() {
        card.clear();
        total = 0;
    }
    // remove a card
    void RemoveCard(card_index_t index, card_count_t count = 1) {
        auto it = std::lower_bound(
            card.begin(), card.end(), deck_item_t(index, 0));
        assert(it != card.end());
        assert(it->second >= count);
        if (it->second == count) {
            card.erase(it);
        } else {
            it->second -= count;
        }
        total -= count;
    }
    // add a card
    void AddCard(card_index_t index, card_count_t count = 1) {
        //card.reserve(card.size() + count);
        auto it = std::lower_bound(
            card.begin(), card.end(), deck_item_t(index, 0));
        if (it == card.end()) {
            card.insert(card.end(), deck_item_t(index, count));
        } else if (it->first == index) {
            it->second += count;
        } else {
            card.insert(it, deck_item_t(index, count));
        }
        total += count;
    }
    // return the number of unique subsets
    std::size_t CountUniqueSubsets() const {
        std::size_t result = 1;
        for (auto & i : card) {
            result *= (std::size_t) i.second + 1;
        }
        return result;
    }
    // convert deck to a human readable form
    string ToString() const {
        std::stringstream ss;
        ss << "{" << (int) total << " cards";
        const Card * last_card = nullptr;
        bool first_card = true;
        for (auto & i : card) {
            if (first_card) {
                ss << ": ";
                first_card = false;
            } else {
                ss << ", ";
            }
            if (i.second > 1) {
                ss << (int) i.second << "x";
            }
            ss << card_map[i.first]->name;
        }
        ss << "}";
        return ss.str();
    }
};
