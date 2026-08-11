#pragma once

#include <cstddef>

namespace synapse::core {

class PoolAllocator {
public:
    PoolAllocator(size_t blockSize, size_t count);
    ~PoolAllocator();

    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;

    void* allocate();
    void deallocate(void* ptr);

private:
    void* m_memory = nullptr;
    void* m_freeHead = nullptr;
    size_t m_blockSize = 0;
    size_t m_count = 0;
    size_t m_used = 0;
};

} // namespace synapse::core
