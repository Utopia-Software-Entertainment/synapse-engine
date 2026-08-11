#include "PoolAllocator.h"

namespace synapse::core {

PoolAllocator::PoolAllocator(size_t size, size_t count) : m_blockSize(size), m_count(count) {}
PoolAllocator::~PoolAllocator() = default;

void* PoolAllocator::allocate() { return nullptr; }
void PoolAllocator::deallocate(void* ptr) {}

} // namespace synapse::core
