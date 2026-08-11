#include "StackAllocator.h"

namespace synapse::core {

StackAllocator::StackAllocator(size_t size) : m_size(size) {}
StackAllocator::~StackAllocator() = default;

void* StackAllocator::allocate(size_t bytes) { return nullptr; }
void StackAllocator::reset() {}

} // namespace synapse::core
