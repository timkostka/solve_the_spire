#pragma once

#include "cards.hpp"

#include <cmath>
#include <utility>
#include <cassert>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string>
#include <iterator>

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
        /*if (total < that.total) {
            return true;
        } else if (total > that.total) {
            return false;
        }
        if (card.size() < that.card.size()) {
            return true;
        } else if (card.size() > that.card.size()) {
            return false;
        }*/
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
    // add a card
    void AddCard(const Card & c, card_count_t count = 1) {
        AddCard(c.GetIndex(), count);
    }
    // add a card by string
    void AddCard(const char * card_name, card_count_t count = 1) {
        for (std::size_t i = 0; i < card_map.size(); ++i) {
            const Card & card = *card_map[i];
            if (card.name == card_name) {
                AddCard(card, count);
                return;
            }
        };
        std::cout << "ERROR: unknown card name" << std::endl;
        assert(false);
    }
    // add a card collection
    void AddCard(const CardCollection & that) {
        for (auto & c : that.card) {
            AddCard(c.first, c.second);
        }
    }
    // return all combinations of selecting X cards at random
    // results are returned as (probability, cards_selected, cards_left)
    std::vector<std::pair<double, std::pair<CardCollection, CardCollection>>> Select(card_count_t count) const {
        // get number of results
        std::vector<std::pair<double, std::pair<CardCollection, CardCollection>>> result;
        card_count_t unique_card_count = (card_count_t) card.size();
        // get number of cards
        card_count_t card_count = total;
        // find probability of this combination
        double denom = ncr(card_count, count);
        // find denominator for probabilities
        // get number of items we can assign past this
        std::vector<card_count_t> space_to_right(unique_card_count, 0);
        if (unique_card_count > 0) {
            std::size_t i = (std::size_t) unique_card_count - 1;
            while (i > 0) {
                --i;
                space_to_right[i] = space_to_right[i + 1] + card[i + 1].second;
            }
        }
        assert(card_count >= count);
        // initialize first list
        std::vector<card_count_t> item(unique_card_count, 0);
        card_count_t left = count;
        card_index_t active_index = 0;
        while (left > 0) {
            item[active_index] = std::min(left, card[active_index].second);
            left -= item[active_index];
            if (left) {
                ++active_index;
            }
        }
        while (true) {
            // add this list
            result.push_back(std::pair<double, std::pair<CardCollection, CardCollection>>());
            auto & this_result = *result.rbegin();
            CardCollection & selected = this_result.second.first;
            CardCollection & remaining = this_result.second.second;
            for (size_t i = 0; i < unique_card_count; ++i) {
                if (item[i] > 0) {
                    selected.AddCard(card[i].first, item[i]);
                }
                if (card[i].second - item[i] > 0) {
                    remaining.AddCard(card[i].first, card[i].second - item[i]);
                }
            }
            // find probability for this subset
            double p = 1;
            for (std::size_t i = 0; i < unique_card_count; ++i) {
                if (item[i] > 0) {
                    p *= ncr(card[i].second, item[i]);
                }
            }
            this_result.first = p / denom;
            // iterate
            if (active_index == unique_card_count - 1) {
                if (active_index == 0) {
                    return result;
                }
                card_index_t need_to_place = item[active_index];
                item[active_index] = 0;
                --active_index;
                while (true) {
                    if (item[active_index] > 0) {
                        --item[active_index];
                        ++need_to_place;
                        if (space_to_right[active_index] >= need_to_place) {
                            break;
                        }
                        need_to_place += item[active_index];
                        item[active_index] = 0;
                        if (active_index == 0) {
                            return result;
                        }
                    }
                    if (active_index == 0) {
                        return result;
                    }
                    --active_index;
                }
                assert(active_index != 255);
                while (need_to_place > 0) {
                    ++active_index;
                    item[active_index] = std::min(need_to_place, card[active_index].second);
                    assert(item[active_index] > 0);
                    need_to_place -= item[active_index];
                }
            } else {
                assert(item[active_index] > 0);
                --item[active_index];
                ++active_index;
                ++item[active_index];
            }
        }
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
