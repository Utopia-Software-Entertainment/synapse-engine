#include <doctest/doctest.h>

#include <core/Memory/MemoryArena.h>
#include <core/Memory/PoolAllocator.h>
#include <core/Memory/StackAllocator.h>

#include <unordered_set>

using namespace synapse::core;

TEST_CASE("MemoryArena: allocation dans la capacite")
{
    MemoryArena arena(1024);
    void* a = arena.allocate(64);
    void* b = arena.allocate(128);
    CHECK(a != nullptr);
    CHECK(b != nullptr);
    CHECK(a != b);
}

TEST_CASE("MemoryArena: allocation au-dela de la capacite renvoie nullptr")
{
    MemoryArena arena(256);
    CHECK(arena.allocate(256) != nullptr);
    CHECK(arena.allocate(1) == nullptr);
}

TEST_CASE("MemoryArena: reset reutilise la memoire")
{
    MemoryArena arena(512);
    void* first = arena.allocate(512);
    arena.reset();
    void* second = arena.allocate(512);
    CHECK(first != nullptr);
    CHECK(second == first);
}

TEST_CASE("PoolAllocator: epuisement puis reutilisation")
{
    PoolAllocator pool(64, 4);
    void* blocks[4];
    for (int i = 0; i < 4; ++i)
    {
        blocks[i] = pool.allocate();
        CHECK(blocks[i] != nullptr);
    }
    CHECK(pool.allocate() == nullptr);

    pool.deallocate(blocks[1]);
    void* reused = pool.allocate();
    CHECK(reused == blocks[1]);
}

TEST_CASE("PoolAllocator: blocs distincts")
{
    PoolAllocator pool(64, 4);
    std::unordered_set<void*> seen;
    for (int i = 0; i < 4; ++i)
    {
        void* p = pool.allocate();
        CHECK(seen.insert(p).second);
    }
}

TEST_CASE("PoolAllocator: deallocate nullptr est sans effet")
{
    PoolAllocator pool(64, 3);
    void* a = pool.allocate();
    pool.deallocate(nullptr);
    void* b = pool.allocate();
    void* c = pool.allocate();
    CHECK(c != nullptr);
    CHECK(a != b);
    CHECK(b != c);
}

TEST_CASE("StackAllocator: allocation dans la capacite")
{
    StackAllocator stack(512);
    void* a = stack.allocate(64);
    void* b = stack.allocate(64);
    CHECK(a != nullptr);
    CHECK(b != nullptr);
    CHECK(a != b);
}

TEST_CASE("StackAllocator: au-dela de la capacite renvoie nullptr")
{
    StackAllocator stack(128);
    CHECK(stack.allocate(128) != nullptr);
    CHECK(stack.allocate(1) == nullptr);
}

TEST_CASE("StackAllocator: reset reutilise la memoire")
{
    StackAllocator stack(256);
    void* first = stack.allocate(256);
    stack.reset();
    void* second = stack.allocate(256);
    CHECK(first != nullptr);
    CHECK(second == first);
}
