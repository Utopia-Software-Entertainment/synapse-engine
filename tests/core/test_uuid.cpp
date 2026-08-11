#include <doctest/doctest.h>

#include <core/UUID.h>

#include <unordered_set>

using namespace synapse::core;

TEST_CASE("UUID: generate produit des valeurs non nulles")
{
    const UUID id = UUID::generate();
    CHECK(id.lo != 0);
    CHECK(id.hi != 0);
}

TEST_CASE("UUID: generate produit des valeurs uniques")
{
    std::unordered_set<uint64_t> seen;
    for (int i = 0; i < 1000; ++i)
    {
        const UUID id = UUID::generate();
        seen.insert(id.lo);
        seen.insert(id.hi);
    }
    CHECK(seen.size() >= 1000);
}

TEST_CASE("UUID: generate differe entre deux appels")
{
    const UUID a = UUID::generate();
    const UUID b = UUID::generate();
    const bool same = (a.lo == b.lo) && (a.hi == b.hi);
    CHECK_FALSE(same);
}
