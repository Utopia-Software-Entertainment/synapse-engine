#include "MemoryArena.h"

#include <cstdlib>
#include <new>

namespace synapse::core {

namespace {

constexpr size_t kAlignment = alignof(std::max_align_t);

}

MemoryArena::MemoryArena(size_t size) : m_size(size)
{
    m_memory = std::malloc(size);
    if (m_memory == nullptr)
    {
        throw std::bad_alloc();
    }
}

MemoryArena::~MemoryArena()
{
    std::free(m_memory);
}

void* MemoryArena::allocate(size_t bytes)
{
    const size_t aligned = (bytes + kAlignment - 1) & ~(kAlignment - 1);
    if (m_offset + aligned > m_size)
    {
        return nullptr;
    }
    void* ptr = static_cast<char*>(m_memory) + m_offset;
    m_offset += aligned;
    return ptr;
}

void MemoryArena::reset()
{
    m_offset = 0;
}

} // namespace synapse::core
