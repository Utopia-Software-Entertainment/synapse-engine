#include "MemoryArena.h"

namespace synapse::core {

MemoryArena::MemoryArena(size_t size) : m_size(size) {}
MemoryArena::~MemoryArena() = default;

void* MemoryArena::allocate(size_t bytes) { return nullptr; }
void MemoryArena::reset() {}

} // namespace synapse::core
