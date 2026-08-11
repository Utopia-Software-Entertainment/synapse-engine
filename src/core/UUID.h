#pragma once

#include <cstdint>

namespace synapse::core {

struct UUID {
    uint64_t lo;
    uint64_t hi;

    UUID() : lo(0), hi(0) {}

    static UUID generate();
};

} // namespace synapse::core
