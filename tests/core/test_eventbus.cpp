#include <doctest/doctest.h>

#include <core/EventBus.h>

using namespace synapse;

struct TestEventA : IEvent
{
    int value = 0;
};

struct TestEventB : IEvent
{
};

TEST_CASE("EventBus: handler recu pour son type")
{
    EventBus bus;
    bus.Init();

    int received = -1;
    bus.RegisterHandler<TestEventA>([&](const IEvent& e) {
        received = static_cast<const TestEventA&>(e).value;
    });

    TestEventA event;
    event.value = 42;
    bus.Emit(event);
    CHECK(received == 42);
}

TEST_CASE("EventBus: plusieurs handlers tous appeles")
{
    EventBus bus;
    int count = 0;
    bus.RegisterHandler<TestEventA>([&](const IEvent&) { ++count; });
    bus.RegisterHandler<TestEventA>([&](const IEvent&) { ++count; });

    bus.Emit(TestEventA{});
    CHECK(count == 2);
}

TEST_CASE("EventBus: les types non enregistres ne declenchent rien")
{
    EventBus bus;
    int count = 0;
    bus.RegisterHandler<TestEventA>([&](const IEvent&) { ++count; });

    bus.Emit(TestEventB{});
    CHECK(count == 0);
}

TEST_CASE("EventBus: Clear supprime les handlers")
{
    EventBus bus;
    int count = 0;
    bus.RegisterHandler<TestEventA>([&](const IEvent&) { ++count; });
    bus.Clear();

    bus.Emit(TestEventA{});
    CHECK(count == 0);
}
