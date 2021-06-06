#pragma once

#include <cstdint>

// buff/debuff type
enum BuffType : uint8_t {
    kBuffStrength,
    kBuffDexterity,
    kBuffWeak,
    kBuffFrail,
    kBuffVulnerable,
    kBuffRitual,
    kBuffThorns,
    kBuffEnrage,
    kBuffMetallicize,
    kBuffCurlUp,
    // at end of turn, decrease strength by this amount and zero this value
    kBuffStrengthDown,
    kBuffRage,
    kBuffFinal,
};

// list of positive buffs
const BuffType positive_buffs[] = {
    kBuffStrength,
    kBuffDexterity,
    kBuffRitual,
    kBuffThorns,
    kBuffEnrage,
    kBuffMetallicize,
    kBuffCurlUp,
    kBuffRage,
};

// list of negative buffs
const BuffType negative_buffs[] = {
    kBuffWeak,
    kBuffFrail,
    kBuffVulnerable,
    kBuffStrengthDown,
};

// list of neutral buffs
//const BuffType ambiguous_buffs[] = {
//};

//// true if buff is favorable on a player
//// e.g. strength is good, vulnerability is bad
//bool favorable_buff[kBuffFinal] = {
//    true, true, false, true, true, true, true
//};

// number of stacks of each buff/debuff
struct BuffState {
    int16_t value[kBuffFinal];
    // reset
    void Reset() {
        memset(value, 0, sizeof(value));
    }
    // default constructor
    BuffState() {
        Reset();
    }
    inline int16_t & operator[] (int16_t buff) {
        return value[buff];
    }
    inline const int16_t & operator[] (int16_t buff) const {
        return value[buff];
    }
    // equality comparison
    bool operator== (const BuffState & that) const {
        return memcmp(value, that.value, sizeof(value)) == 0;
    }
    // inequality comparison
    bool operator!= (const BuffState & that) const {
        return !(*this == that);
    }
    // return true if player buffs are worse or equal
    bool PlayerIsWorseOrEqual(const BuffState & that) const {
        if (*this == that) {
            return true;
        }
        for (const auto & buff : positive_buffs) {
            if (value[buff] > that.value[buff]) {
                return false;
            }
        }
        for (const auto & buff : negative_buffs) {
            if (value[buff] < that.value[buff]) {
                return false;
            }
        }
        return true;
    }
    // return true if mob buffs are worse or equal
    bool MobIsWorseOrEqual(const BuffState & that) const {
        if (*this == that) {
            return true;
        }
        for (const auto & buff : positive_buffs) {
            if (value[buff] < that.value[buff]) {
                return false;
            }
        }
        for (const auto & buff : negative_buffs) {
            if (value[buff] > that.value[buff]) {
                return false;
            }
        }
        return true;
    }
    // cycle buffs
    void Cycle() {
        if (value[kBuffVulnerable]) {
            --value[kBuffVulnerable];
        }
        if (value[kBuffWeak]) {
            --value[kBuffWeak];
        }
        if (value[kBuffFrail]) {
            --value[kBuffFrail];
        }
        if (value[kBuffRitual]) {
            value[kBuffStrength] += value[kBuffRitual];
        }
        if (value[kBuffStrengthDown]) {
            value[kBuffStrength] -= value[kBuffStrengthDown];
            value[kBuffStrengthDown] = 0;
        }
    }
    // convert this to string form
    std::string ToString() const {
        std::ostringstream ss;
        ss << "Buff(";
        bool first = true;
        if (value[kBuffStrength]) {
            if (!first) {
                ss << ", ";
            }
            first = false;
            //if ()
            //ss << "s"
        }
        ss << ")";
        return ss.str();
    }
};
