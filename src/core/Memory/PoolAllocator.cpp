#include "PoolAllocator.h"

#include <cstdlib>
#include <new>

namespace synapse::core {

PoolAllocator::PoolAllocator(size_t blockSize, size_t count) : m_blockSize(blockSize), m_count(count)
{
    if (blockSize < sizeof(void*))
    {
        m_blockSize = sizeof(void*);
    }
    m_memory = std::malloc(m_blockSize * count);
    if (m_memory == nullptr)
    {
        throw std::bad_alloc();
    }
    m_freeHead = m_memory;
    for (size_t i = 0; i < count; ++i)
    {
        void* block = static_cast<char*>(m_memory) + i * m_blockSize;
        void* next = (i + 1 < count) ? static_cast<char*>(block) + m_blockSize : nullptr;
        *static_cast<void**>(block) = next;
    }
}

PoolAllocator::~PoolAllocator()
{
    std::free(m_memory);
}

void* PoolAllocator::allocate()
{
    if (m_freeHead == nullptr)
    {
        return nullptr;
    }
    void* block = m_freeHead;
    m_freeHead = *static_cast<void**>(block);
    ++m_used;
    return block;
}

void PoolAllocator::deallocate(void* ptr)
{
    if (ptr == nullptr)
    {
        return;
    }
    *static_cast<void**>(ptr) = m_freeHead;
    m_freeHead = ptr;
    --m_used;
}

} // namespace synapse::core
