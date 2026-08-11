#include "UUID.h"

#include <random>

namespace synapse::core {

UUID UUID::generate()
{
    static thread_local std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<uint64_t> dist;

    UUID id;
    id.lo = dist(rng);
    id.hi = dist(rng);
    return id;
}

} // namespace synapse::core
