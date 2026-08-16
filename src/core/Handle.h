#pragma once

#include <core/Types.h>

namespace synapse {

// Generational handle: index + generation packed in a u32, so a stale handle
// (freed slot reused by another object) can be detected. `Tag` keeps handle
// types distinct at compile time (asset vs entity vs mesh, ...).
//
// Layout: [index : 20 bits][generation : 12 bits] → up to ~1M live handles,
// 4095 generations before wrap.
template <typename Tag>
class Handle
{
public:
    static constexpr u32 kIndexBits     = 20;
    static constexpr u32 kGenerationBits = 32 - kIndexBits;
    static constexpr u32 kIndexMask     = (1u << kIndexBits) - 1;
    static constexpr u32 kMaxGeneration = (1u << kGenerationBits) - 1;
    static constexpr u32 kInvalidPacked = 0xFFFFFFFFu;

    Handle() = default;

    Handle(u32 index, u32 generation)
        : m_Packed((index & kIndexMask) | ((generation & kMaxGeneration) << kIndexBits)) {}

    u32 Index() const      { return m_Packed & kIndexMask; }
    u32 Generation() const { return (m_Packed >> kIndexBits) & kMaxGeneration; }
    u32 Packed() const     { return m_Packed; }

    bool IsValid() const { return m_Packed != kInvalidPacked; }

    void Invalidate() { m_Packed = kInvalidPacked; }

    static Handle Invalid() { return Handle(); }

    bool operator==(const Handle& other) const { return m_Packed == other.m_Packed; }
    bool operator!=(const Handle& other) const { return m_Packed != other.m_Packed; }
    bool operator<(const Handle& other) const  { return m_Packed < other.m_Packed; }

private:
    u32 m_Packed = kInvalidPacked;
};

} // namespace synapse