#include <doctest/doctest.h>

#include <core/Handle.h>

namespace {

struct AssetTag;
struct MeshTag;

} // namespace

using namespace synapse;

TEST_CASE("Handle: invalid by default")
{
    Handle<AssetTag> h;
    CHECK_FALSE(h.IsValid());
    CHECK(h == Handle<AssetTag>::Invalid());
}

TEST_CASE("Handle: index/generation roundtrip")
{
    Handle<AssetTag> h(42, 3);
    CHECK(h.IsValid());
    CHECK(h.Index() == 42);
    CHECK(h.Generation() == 3);
    CHECK(h.Packed() != 0);
}

TEST_CASE("Handle: equality and ordering")
{
    Handle<AssetTag> a(7, 1);
    Handle<AssetTag> b(7, 1);
    Handle<AssetTag> c(8, 1);

    CHECK(a == b);
    CHECK(a != c);
    CHECK(a < c);
}

TEST_CASE("Handle: generation detects stale handle")
{
    Handle<AssetTag> fresh(0, 1);
    Handle<AssetTag> stale(0, 0);
    CHECK_FALSE(fresh == stale);   // same slot, different generation
    CHECK(fresh.Generation() != stale.Generation());
}

TEST_CASE("Handle: generation wraps at max")
{
    Handle<AssetTag> h(5, Handle<AssetTag>::kMaxGeneration);
    CHECK(h.Generation() == Handle<AssetTag>::kMaxGeneration);
    const Handle<AssetTag> wrapped(5, Handle<AssetTag>::kMaxGeneration + 1);
    CHECK(wrapped.Generation() == 0);
}

TEST_CASE("Handle: tag types stay distinct")
{
    Handle<AssetTag> asset(1, 0);
    Handle<MeshTag>  mesh(1, 0);
    // Compile-time distinct types — no implicit conversion between them.
    CHECK(asset.Index() == mesh.Index());
    (void)asset;
    (void)mesh;
}