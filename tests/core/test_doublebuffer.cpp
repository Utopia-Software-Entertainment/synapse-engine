#include <doctest/doctest.h>

#include <core/DoubleBuffer.h>

#include <thread>

using namespace synapse;

struct Payload
{
    u32 value = 0;
    u32 checksum = 0;
};

TEST_CASE("DoubleBuffer: write/commit/read roundtrip")
{
    DoubleBuffer<Payload> db;

    db.Write().value = 7;
    db.Commit();

    CHECK(db.Read().value == 7);
}

TEST_CASE("DoubleBuffer: reader never sees torn intermediate writes")
{
    DoubleBuffer<Payload> db;

    // Simulate a producer writing all fields, then committing atomically.
    for (u32 i = 0; i < 1000; ++i)
    {
        Payload& p = db.Write();
        p.value = i;
        p.checksum = i * 31u;
        db.Commit();
        CHECK(db.Read().checksum == db.Read().value * 31u);
    }
}

TEST_CASE("DoubleBuffer: writer and reader buffers are distinct")
{
    DoubleBuffer<Payload> db;

    Payload& pending = db.Write();
    pending.value = 99;

    // Not committed yet → reader still sees the initial committed buffer.
    CHECK(db.Read().value == 0);
    CHECK(&pending != &db.Read());
}

TEST_CASE("DoubleBuffer: producer/consumer across threads")
{
    DoubleBuffer<Payload> db;
    std::atomic<bool> stop{false};

    std::thread producer([&]() {
        for (u32 i = 1; !stop.load(); ++i)
        {
            Payload& p = db.Write();
            p.value = i;
            p.checksum = i * 2u;
            db.Commit();
            std::this_thread::yield();
        }
    });

    u32 lastValue = 0;
    std::thread consumer([&]() {
        for (u32 i = 0; i < 2000; ++i)
        {
            const Payload p = db.Read();
            lastValue = p.value;
            CHECK(p.checksum == p.value * 2u);   // snapshot is consistent
        }
    });

    consumer.join();
    stop.store(true);
    producer.join();

    CHECK(lastValue > 0);
}