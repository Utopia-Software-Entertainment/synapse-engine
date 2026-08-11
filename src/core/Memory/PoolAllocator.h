#pragma once

#include <cstddef>

namespace synapse::core {

class PoolAllocator {
public:
    PoolAllocator(size_t blockSize, size_t count);
    ~PoolAllocator();
    void* allocate();
    void deallocate(void* ptr);
private:
    size_t m_blockSize;
    size_t m_count;
};

} // namespace synapse::core
