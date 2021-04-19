#pragma once

#include <cstdint>
#include <vector>

// orb types
enum OrbEnum {
    kOrbLightning,
    kOrbFrost,
    kOrbFusion,
    kOrbDark,
};

struct OrbStruct {
    // type of orb
    OrbEnum type;
    // dark orb damage count
    uint16_t damage;
    // constructor
    OrbStruct(OrbEnum type) : type(type), damage(0) {
    }
    // convert to string representation
    std::string ToString() const {
        if (type == kOrbLightning) {
            return "Lightning";
        } else if (type == kOrbFrost) {
            return "Frost";
        } else if (type == kOrbDark) {
            std::string result;
            result = "Dark(" + std::to_string(damage) + ")";
            return result;
        } else {
            assert(type == kOrbFusion);
            return "Fusion";
        }
    }
};

struct OrbsStruct {
    // orb capacity
    uint8_t capacity;
    // orb list
    std::vector<OrbStruct> orb;
};
