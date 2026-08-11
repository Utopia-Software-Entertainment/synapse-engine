#include "StackAllocator.h"

#include <cstdlib>
#include <new>

namespace synapse::core {

StackAllocator::StackAllocator(size_t size) : m_size(size)
{
    m_memory = std::malloc(size);
    if (m_memory == nullptr)
    {
        throw std::bad_alloc();
    }
}

StackAllocator::~StackAllocator()
{
    std::free(m_memory);
}

void* StackAllocator::allocate(size_t bytes)
{
    if (m_offset + bytes > m_size)
    {
        return nullptr;
    }
    void* ptr = static_cast<char*>(m_memory) + m_offset;
    m_offset += bytes;
    return ptr;
}

void StackAllocator::reset()
{
    m_offset = 0;
}

} // namespace synapse::core
