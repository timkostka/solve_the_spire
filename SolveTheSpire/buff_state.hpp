#pragma once

#include <cstdint>

// buff/debuff type
enum BuffType : uint8_t {
    kBuffStrength,
    kBuffDexterity,
    kBuffVulnerable,
    kBuffRitual,
    kBuffThorns,
    kBuffEnrage,
    kBuffMetallicize,
    kBuffFinal,
};

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
    const int16_t & operator[] (int16_t buff) const {
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
    // cycle buffs
    void Cycle() {
        if (value[kBuffVulnerable]) {
            --value[kBuffVulnerable];
        }
        if (value[kBuffRitual]) {
            value[kBuffStrength] += value[kBuffRitual];
        }
    }
};
