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
    bool operator < (const CardCollectionNode & that) const {
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
            node_ptr->add_card_node.resize((std::size_t) index + 1, nullptr);
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
            new_node_ptr->remove_card_node.resize((std::size_t) index + 1, nullptr);
            new_node_ptr->remove_card_node[index] = node_ptr;
            node_ptr = new_node_ptr;
        } else {
            // already exists, so just update add/remove pointers
            const CardCollectionNode * new_node_ptr = &*it;
            node_ptr->add_card_node[index] = new_node_ptr;
            if (new_node_ptr->remove_card_node.size() <= index) {
                new_node_ptr->remove_card_node.resize((std::size_t) index + 1, nullptr);
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
            node_ptr->remove_card_node.resize((std::size_t) index + 1, nullptr);
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
            new_node_ptr->add_card_node.resize((std::size_t) index + 1, nullptr);
            new_node_ptr->add_card_node[index] = node_ptr;
            node_ptr = new_node_ptr;
        } else {
            // already exists, so just update add/remove pointers
            const CardCollectionNode * new_node_ptr = &*it;
            node_ptr->remove_card_node[index] = new_node_ptr;
            if (new_node_ptr->add_card_node.size() <= index) {
                new_node_ptr->add_card_node.resize((std::size_t) index + 1, nullptr);
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
    // upgrade a specific card
    void UpgradeCard(const Card & card, card_count_t count = 1) {
        RemoveCard(card, count);
        AddCard(*card.upgraded_version, count);
    }
    // upgrade all cards that can be upgraded
    void UpgradeAll() {
        for (auto & deck_item : *this) {
            auto index = deck_item.first;
            auto count = deck_item.second;
            const auto & card = *card_map[index];
            if (card.upgraded_version) {
                AddCard(*card.upgraded_version, count);
                RemoveCard(index, count);
            }
        }
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
    // sort by least probable first
    static bool LeastProbableSort(
            const std::pair<double, std::pair<CardCollectionPtr, CardCollectionPtr>> & one,
            const std::pair<double, std::pair<CardCollectionPtr, CardCollectionPtr>> & two) {
        return one.first < two.first;
    }
    // return all combination of selecting X cards at random without replacement
    // results are returned as (probability, cards_selected, cards_left)
    std::vector<std::pair<double, std::pair<CardCollectionPtr, CardCollectionPtr>>> *
            Select(card_count_t count) const {
        // cache results
        static std::map<const std::pair<const CardCollectionNode * const, card_count_t>, std::vector<std::pair<double, std::pair<CardCollectionPtr, CardCollectionPtr>>> *>
            selection_cache;
        // look for cached result
        const std::pair<const CardCollectionNode * const, card_count_t>
            key(node_ptr, count);
        auto it = selection_cache.find(key);
        if (it != selection_cache.end()) {
            return it->second;
        }
        // hold results
        const auto & card = node_ptr->collection.card;
        std::vector<std::pair<double, std::pair<CardCollectionPtr, CardCollectionPtr>>> * result;
        result = new std::vector<std::pair<double, std::pair<CardCollectionPtr, CardCollectionPtr>>>();
        card_count_t unique_card_count = (card_count_t) card.size();
        // get number of cards
        card_count_t card_count = node_ptr->collection.total;
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
        bool done = false;
        while (true) {
            // add this list
            result->push_back(std::pair<double, std::pair<CardCollectionPtr, CardCollectionPtr>>());
            auto & this_result = *result->rbegin();
            CardCollectionPtr & selected = this_result.second.first;
            CardCollectionPtr & remaining = this_result.second.second;
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
                    break;
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
                            done = true;
                            break;
                        }
                    }
                    if (active_index == 0) {
                        done = true;
                        break;
                    }
                    --active_index;
                }
                if (done) {
                    break;
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
        // sort by lease probable first
        std::sort(result->begin(), result->end(), LeastProbableSort);
        // cache result
        selection_cache[key] = result;
        // return result
        return result;
    }
};

// empty card collection
CardCollectionNode CardCollectionPtr::empty_node;

// set of all current card collections
std::set<CardCollectionNode> CardCollectionPtr::deck_collection;
